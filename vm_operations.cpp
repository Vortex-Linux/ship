#include "ship.h"

void pass_password_to_tmux() {
    std::string capture_tmux_last_line_cmd = "tmux capture-pane -p -S -1 -t " + ship_env.name + " | tail -n 1";

    sleep(1);

    while (exec((capture_tmux_last_line_cmd)).find("password for") != std::string::npos) {
        std::cout << "Please provide your root password: ";     
        std::string root_password;     
        std::getline(std::cin, root_password); 
 
        system_command_vm(root_password);

        sleep(1);
    }
}

void wait_for_vm_ready() {
    while(true) {
        std::string capture_tmux_last_line_cmd = "tmux capture-pane -p -S -1 -t " + ship_env.name + " | tail -n 1";
        std::string output = exec(capture_tmux_last_line_cmd);

        output = trim_trailing_whitespaces(output);

        if (!output.empty()) {
            break;
        }
        sleep(1);
    }
}

void run_startup_commands() {
    boost::property_tree::ini_parser::read_ini(find_settings_file(), pt);

    bool system_exec_commands_left = true;
    bool exec_commands_left = true;
    int current_command_number = 1;

    while(system_exec_commands_left || exec_commands_left) {
        try {
            std::string current_startup_system_command = pt.get<std::string>("system_exec.command_" + std::to_string(current_command_number));
            system_command_vm(current_startup_system_command);
        } catch(const boost::property_tree::ptree_error& e) {
            system_exec_commands_left = false;
        }

        sleep(1);

        try {
          std::string current_startup_exec_command = pt.get<std::string>("exec.command_" + std::to_string(current_command_number));
            exec_command_vm(current_startup_exec_command);
        } catch(const boost::property_tree::ptree_error& e) {
            exec_commands_left = false;
        } 

        sleep(1);

        current_command_number += 1;
    }
}

std::string find_network_address_vm() {
    std::string find_network_address_vm_cmd = "virsh domifaddr " + ship_env.name + " | awk '{print $4}' | cut -d'/' -f1 | tail -n 2";
    return trim_trailing_whitespaces(exec(find_network_address_vm_cmd)); 
}

void attach_xpra(const std::string &username,const std::string &password) {
    int max_retries = 5; 
    int delay = 10;
    int attempt = 1;
    
    std::cout << "Trying to connect to the xpra server. This might take a few moments if the server isn't running yet." << std::endl;

    std::string xpra_attach_success_message = "Attached to xpra server";
    std::string xpra_attach_failure_message = "removing unix domain socket";

    std::string attach_xpra_server_cmd = "nohup xpra attach ssh://" + username + ":" + password + "@" + find_network_address_vm() + "/100 > /tmp/xpra_attach.log 2>&1 & disown";

    while (attempt <= max_retries) {
        std::string delete_old_xpra_attach_log_cmd = "rm /tmp/xpra_attach.log"; 
        system(delete_old_xpra_attach_log_cmd.c_str()); 
        
        system(attach_xpra_server_cmd.c_str());

        std::string xpra_attach_log_path = "/tmp/xpra_attach.log";
        if (wait_for_file(xpra_attach_log_path, 10) && wait_for_file_to_fill(xpra_attach_log_path, 10)) { 
            bool xpra_attach_message_found = false;
            while (!xpra_attach_message_found) {
                sleep(5);
                std::ifstream file(xpra_attach_log_path);
                if (file.is_open()) {
                    std::string line; 
                    while (getline(file, line)) { 
                        if (line.find(xpra_attach_success_message) != std::string::npos) {
                            std::cout << "Successfully attached to the xpra session on VM" << std::endl;
                            return;
                        }

                        if (line.find(xpra_attach_failure_message) != std::string::npos) {
                            xpra_attach_message_found = true;
                            break;
                        }
                    }
                } else {
                    std::cerr << "Failed to open log file. Check permissions." << std::endl;
                    exit(EXIT_FAILURE);
                }  
            }
        } else {
            std::cout << "Xpra attach log file didnt appear on time." << std::endl;
            exit(EXIT_FAILURE);
        } 

        std::cout << "Attempt " << attempt << " of " << max_retries << " failed. Server might not be running yet." << std::endl;
        sleep(delay); 
        attempt++;

    }

    std::cout << "Could not attach to the xpra server after " << max_retries << " attempts. Please check if the server is running." << std::endl;
    exit(EXIT_FAILURE);
} 

void start_vm() {
    std::string load_saved_cmd = "virsh snapshot-revert --current " + ship_env.name;
    exec(load_saved_cmd);

    std::string state = get_vm_state(ship_env.name);

    if (state.find("running") == std::string::npos) {
          std::string start_cmd = "virsh start " + ship_env.name;
          exec(start_cmd);
    }

    std::cout << "VM " << ship_env.name << " started successfully.\n";

    std::string kill_tmux_session_if_exist_cmd = "tmux has-session -t " + ship_env.name + " && tmux kill-session -t " + ship_env.name + " || true";
    system_exec(kill_tmux_session_if_exist_cmd);

    std::string create_tmux_session_cmd = "tmux new -d -s " + ship_env.name;
    system_exec(create_tmux_session_cmd);

    std::string set_virsh_system_uri_cmd = "export LIBVIRT_DEFAULT_URI=qemu:///system";
    system_command_vm(set_virsh_system_uri_cmd); 

    std::string vm_console_cmd = "virsh console " + ship_env.name;
    system_command_vm(vm_console_cmd);

    pass_password_to_tmux();

    wait_for_vm_ready();
    
    run_startup_commands();

    try {
        boost::property_tree::ini_parser::read_ini(find_settings_file(), pt);

        if(!pt.get<bool>("xpra.enabled", false)) {
            return;
        }

        std::string start_xpra_server_cmd = "xpra start :100";
        exec_command_vm(start_xpra_server_cmd); 

        std::string username = pt.get<std::string>("credentials.username");
        std::string password = pt.get<std::string>("credentials.password");
        
        attach_xpra(username,password);

    } catch(const boost::property_tree::ptree_error& e) {
        std::cout << "This VM cannot perform application forwarding because the credentials are not set correctly. Please reference the documentation to add the credentials if you believe Xpra is already set up for application forwarding, or set it up manually if nothing is configured yet." << std::endl;
    }
}

void restart_vm() {
    shutdown_vm();
    start_vm();
}

void view_vm_console() {
    std::string view_cmd = "tmux attach-session -t " + ship_env.name;      
    system_exec(view_cmd);
}

void view_vm_gui() {
    std::string view_cmd = "virt-viewer " + ship_env.name; 
    system_exec(view_cmd);
}

void view_vm() {
    std::cout << "Do you want a full GUI of the VM(By default the view action will show only a terminal of the VM) ? (y/n): ";     
    std::string confirm;     
    std::getline(std::cin, confirm); 

    if (confirm != "y" && confirm != "Y") {
        view_vm_console();
    }else {
        view_vm_gui();
    }   
}

void pause_vm() {
    std::string view_cmd = "virsh suspend " + ship_env.name; 
    system_exec(view_cmd);
}

void resume_vm() {
    std::string view_cmd = "virsh resume " + ship_env.name; 
    system_exec(view_cmd);
}

void delete_old_snapshots() {
    std::string list_snapshot_cmd = "virsh snapshot-list --name " + ship_env.name;
    std::string snapshot_list = exec(list_snapshot_cmd);
    std::vector<std::string> snapshots = split_string_by_line(snapshot_list);

    std::reverse(snapshots.begin(), snapshots.end());

    std::string latest_snapshot = snapshots.front();

    for (const auto& snapshot : snapshots) {
        if (snapshot != latest_snapshot) {
            std::cout << "Deleting old snapshot: " << snapshot << std::endl;
            std::string delete_cmd = "virsh snapshot-delete " + ship_env.name + " " + snapshot;
            system_exec(delete_cmd);
        }
    }

    std::cout << "Kept latest snapshot: " << latest_snapshot << std::endl;
}

void save_vm() {
    std::string save_cmd = "virsh snapshot-create --atomic " + ship_env.name; 
    system_exec(save_cmd);

    delete_old_snapshots();
}

void shutdown_vm() {
    std::string state = get_vm_state(ship_env.name);
    if (state.find("running") == std::string::npos && state.find("paused") == std::string::npos) {
        std::cout << "The vm is already in a shut-off state" << std::endl;

        if (ship_env.action == ShipAction::RESTART) {
            std::cout << "Cancelling the restart proccess" << std::endl;
            exit(EXIT_FAILURE);
        }

        return;
    }

    std::string shutdown_cmd = "virsh shutdown " + ship_env.name; 
    system_exec(shutdown_cmd); 
    
    std::string shutdown_signal = "virsh event --event lifecycle --timeout 5 " + ship_env.name;
    bool timeout = system(shutdown_signal.c_str());
    if (timeout) {
        std::cout << "The VM isnt responding do you want to forcefully shutdown the vm ? (y/n): ";
        std::string confirm;
        std::getline(std::cin, confirm);

        if (confirm != "y" && confirm != "Y") {
            std::cout << "VM force shutdown proccess cancelled for " << ship_env.name << "'.\n";
            return;
        }

        std::cout << "Forcefully shutting down " << ship_env.name << ".\n";
        std::string shutdown_cmd = "virsh destroy " + ship_env.name + "\n";
        system_exec(shutdown_cmd);
    }
    std::cout << "Successfully shutdown " << ship_env.name << std::endl; 
}

std::string get_vm_image_paths() {
    std::string get_vm_image_paths_cmd = "virsh domblklist " + ship_env.name + " --details | awk '{print $4}' | tail -n +3 | head -n -1";
    std::string image_paths = exec(get_vm_image_paths_cmd);
    return image_paths;
}

void clean_vm_resources() {
    std::cout << "Deleting all resources which are not needed anymore." << std::endl;;

    std::string image_paths = get_vm_image_paths();
    std::stringstream stream(image_paths);
    std::string image_path;

    while (std::getline(stream, image_path)) {
        std::cout << "Deleting " << image_path << std::endl;
        std::string delete_vm_image_cmd = "rm " + image_path;
        system(delete_vm_image_cmd.c_str());
    }

    std::string delete_vm_settings_cmd = "rm " + ship_lib_path + "settings/vm-settings/" + ship_env.name + ".ini";
    system(delete_vm_settings_cmd.c_str());

    std::cout << "Successfully deleted all resources which are not needed anymore." << std::endl;;
}

bool vm_exists(const std::string& vm_name) {
    std::string check_cmd = "virsh list --all | grep -w " + vm_name;
    int result = system(check_cmd.c_str());
    return result == 0;  
}

void delete_vm() {
    std::string state = get_vm_state(ship_env.name);

    if (!vm_exists(ship_env.name)) {
        std::cout << "VM " << ship_env.name << " does not exist" << std::endl;
        std::cout << "Cancelling the deletion proccess" << std::endl;
        return;
    }

    if (state.find("running") != std::string::npos || state.find("paused") != std::string::npos) {
        std::cout << "forcefully shutting down vm " << ship_env.name << " before deletion.\n";
        std::string shutdown_cmd = "virsh destroy " + ship_env.name;
        system_exec(shutdown_cmd);
    }

    clean_vm_resources();

    std::string undefine_cmd = "virsh undefine --nvram " + ship_env.name;
    exec(undefine_cmd);
    std::cout << "VM " << ship_env.name << " deleted successfully.\n";

}

void process_source_file() {
    if (ship_env.source_local.find_last_of(".") != std::string::npos) {
        std::string extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));

        if (extension == ".gz") {
            ship_env.source_local = decompress_gzip_file(ship_env.source_local);
            extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));
        } else if (extension == ".bz2") {
            ship_env.source_local = decompress_bzip2_file(ship_env.source_local);
            extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));
        } else if (extension == ".xz") {
            ship_env.source_local = decompress_xz_file(ship_env.source_local);
            extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));
        } else if (extension == ".lz4") {
            ship_env.source_local = decompress_lz4_file(ship_env.source_local);
            extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));
        } else if (extension == ".lzo") {
            ship_env.source_local = decompress_lzo_file(ship_env.source_local);
            extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));
        } else if (extension == ".lzma") {
            ship_env.source_local = decompress_lzma_file(ship_env.source_local);
            extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));
        } else if (extension == ".lz") {
            ship_env.source_local = decompress_lzip_file(ship_env.source_local);
            extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));
        }

        if (extension == ".iso") {
            ship_env.iso_path = get_absolute_path(ship_env.source_local);
            create_disk_image();
        }else if (extension == ".qcow2" || extension == ".qcow" || extension == ".img") {
            ship_env.disk_image_path = get_absolute_path(ship_env.source_local);
        }
    }
}
void create_vm() {
    
    bool custom_vm = true;

    if (ship_env.name.empty()) {
        generate_vm_name();
    }

    if (ship_env.name.empty()) {
      std::cout << "Please specify the name of this VM(leaving this blank will set the name to the name given by the sender which is " << ship_env.name << ")";
        std::string name_given;
        std::getline(std::cin, name_given);
        if(!name_given.empty()) {
            ship_env.name = name_given; 
        }
    }

    if (vm_exists(ship_env.name)) {
        std::cout << "VM with the name " << ship_env.name << " already exists" << std::endl;
        std::cout << "Do you want to replace VM " << ship_env.name << " ? (y/n): ";

        std::string confirm;
        std::getline(std::cin, confirm);

        if (confirm == "y" || confirm == "Y") {
            std::cout << "Deleting " << ship_env.name << " before creating new one" << std::endl;
            delete_vm();
        } else {
            std::cout << "VM creation proccess cancelled" << std::endl;
            return;
        }
    }

    std::cout << "Creating new virtual machine for ship with name " + ship_env.name + "\n";

    if (ship_env.source.empty() && ship_env.source_local.empty()) {
        custom_vm = false;
        get_tested_vm();
    }

    if (ship_env.source_local.empty() && ship_env.source.empty()) {
        std::cout << "Ship failed to create a VM because no valid source was specified.\n";
        exit(0);
    }

    get_iso_source();

    ship_env.source_local = trim_trailing_whitespaces(ship_env.source_local);


    set_memory_limit();
    set_cpu_limit();

    std::string xml_filename = generate_vm_xml();
    define_vm(xml_filename);

    if (!custom_vm) {
        configure_vm();
    } else {
        start_vm_with_confirmation_prompt();
}

}

std::string generate_vm_xml() {
    std::stringstream vm_xml;
    vm_xml << R"(
<domain type='kvm'>
  <name>)" << ship_env.name << R"(</name>
  <memory unit='MiB'>)" << ship_env.memory_limit << R"(</memory>
  <vcpu placement='static'>)" << ship_env.cpu_limit << R"(</vcpu>
  <os>
    <type arch='x86_64' machine='pc-i440fx-2.9'>hvm</type>
    <boot dev='hd'/>
    <boot dev='cdrom'/>
    <kernel commandline="quiet loglevel=0"/>
  </os>
  <features>
    <acpi/>
    <apic/>
    <hyperv>
      <relaxed state='on'/>
      <vapic state='on'/>
      <spinlocks state='on' retries='8191'/>
    </hyperv>
  </features>
  <clock offset='localtime'>
    <timer name='rtc' tickpolicy='catchup'/>
    <timer name='pit' tickpolicy='delay'/>
    <timer name='hpet' present='no'/>
    <timer name='hypervclock' present='yes'/>
  </clock>
  <devices>
    <disk type='file' device='disk'>
      <driver name='qemu' type='qcow2'/>
      <source file=')" << ship_env.disk_image_path << R"('/>
      <target dev='vda' bus='virtio'/>
    </disk>)";

    if (!ship_env.iso_path.empty()) {
        vm_xml << R"(
    <disk type='file' device='cdrom'>
      <driver name='qemu' type='raw'/>
      <source file=')" << ship_env.iso_path << R"('/>
      <target dev='sda' bus='sata'/>
      <readonly/>
    </disk>)";
    }

    vm_xml << R"(
    <controller type='usb' index='0'>
      <model name='qemu-xhci'/>
    </controller>
    <controller type='pci' index='0' model='pci-root'/>
    <controller type='sata' index='0'/>
    <interface type='network'>
      <mac address=')" << generate_mac_address() << R"('/>
      <source network='default'/>
      <model type='virtio'/>
    </interface>
    <input type='tablet' bus='usb'/>
    <input type='keyboard' bus='usb'/>
    <graphics type='vnc' port='-1' autoport='yes' listen='0.0.0.0'>
      <listen type='address' address='0.0.0.0'/>
    </graphics>
    <sound model='ich9'/>
    <video>
      <model type='qxl' vram='65536' heads='1'/>
    </video>
    <memballoon model='virtio'/>
)";
    switch(ship_env.os) {
        case TestedVM::gentoo:
        case TestedVM::ubuntu:
        case TestedVM::fedora:
        case TestedVM::freebsd:
        case TestedVM::openbsd:
        case TestedVM::netbsd:
        case TestedVM::dragonflybsd:
        case TestedVM::windows:
        case TestedVM::debian:
        case TestedVM::centos:
        case TestedVM::alpine:
        case TestedVM::whonix:
        case TestedVM::tails:
            vm_xml << R"(
      <console type='pty'>
        <target type='virtio'/>
      </console>
      <serial type='pty'>
        <target port='0'/>
      </serial>
            )";
            break;
        case TestedVM::arch:
            vm_xml << R"(
      <console type='pty'>
        <target type='virtio'/>
      </console>
            )";
            break;
        default:
            break;
    }

    vm_xml << R"(
  </devices>
</domain>
    )";


    std::string xml_filename = "/tmp/" + ship_env.name + ".xml";
    std::ofstream xml_file(xml_filename);
    xml_file << vm_xml.str();
    xml_file.close();

    std::string create_vm_settings_cmd = "touch " + ship_lib_path + "settings/vm-settings/" + ship_env.name + ".ini";
    system_exec(create_vm_settings_cmd);

    return xml_filename;
}

void define_vm(const std::string& xml_filename) {
    std::string define_cmd = "virsh define " + xml_filename;
    exec(define_cmd);

    std::cout << "New VM configuration defined.\n";
}

void start_vm_with_confirmation_prompt() {
    std::cout << "Do you want to start the VM " << ship_env.name << " right now? (y/n): ";
    std::string confirm;
    std::getline(std::cin, confirm);

    if (confirm == "y" || confirm == "Y") {
        std::cout << "Starting VM " << ship_env.name << "...\n";
    } else {
      std::cout << "VM has not been started. You can start it later with 'ship start " << ship_env.name << "'.\n";
        return;
    }
    start_vm();
}

void get_iso_source() {
    if (!ship_env.source.empty()) {
        std::cout << "Downloading iso to images" << "\n";
        
        std::string download_cmd = "aria2c --dir " + ship_lib_path + "images/iso-images " + ship_env.source;
        system_exec(download_cmd);
        
        std::cout << "Finding the path to the downloaded iso image" << "\n";
        
        std::string find_latest_image_cmd = "find " + ship_lib_path + "images/iso-images -type f -exec ls -t1 {} + | head -1";
        ship_env.source_local = exec(find_latest_image_cmd);
        
        std::cout << "Found the path as " << ship_env.source_local << "\n";
    }
}

void print_available_tested_vms() {
    std::cout << "The available tested and configured vms are: " << std::endl;
    std::cout << "tails" << std::endl;
    std::cout << "whonix" << std::endl;
    std::cout << "debian" << std::endl;
    std::cout << "ubuntu" << std::endl;
    std::cout << "arch" << std::endl;
    std::cout << "gentoo" << std::endl;
    std::cout << "fedora" << std::endl;
    std::cout << "centos" << std::endl;
    std::cout << "alpine" << std::endl;
    std::cout << "freebsd" << std::endl;
    std::cout << "openbsd" << std::endl;
    std::cout << "netbsd" << std::endl;
    std::cout << "dragonflybsd" << std::endl;
    std::cout << "windows" << std::endl;

}

std::string get_tested_vm_link(const std::string &vm_name) {
    std::cout << ship_env.source << std::endl;
    if (vm_name == "tails") {
        return "";
    } else if (vm_name == "whonix") {
        return "";
    } else if (vm_name == "debian") {
        return "";
    } else if (vm_name == "ubuntu") {
        return "";
    } else if (vm_name == "arch") {
        return "https://github.com/Vortex-Linux/Arch-VM-Base/releases/download/v0.1.2/archlinux.qcow2.xz";
    } else if (vm_name == "gentoo") {
        return "";
    } else if (vm_name == "fedora") {
        return "";
    } else if (vm_name == "alpine") {
        return "";
    } else if (vm_name == "centos") {
        return "";
    } else if (vm_name == "freebsd") {
        return "";
    } else if (vm_name == "openbsd") {
        return "";
    } else if (vm_name == "netbsd") {
        return "";
    } else if (vm_name == "dragonflybsd") {
        return "";
    } else if (vm_name == "windows") {
        return "";
    }
    return "";
}

void tested_vm_information() {
    switch(ship_env.os) {
        case TestedVM::tails:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::whonix:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::debian:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::ubuntu:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::arch:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::gentoo:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::fedora:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::alpine:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::centos:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::freebsd:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::openbsd:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::netbsd:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::dragonflybsd:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        case TestedVM::windows:
            std::cout << "Hostname:archlinux," << std::endl;
            std::cout << "Username:arch," << std::endl; 
            std::cout << "Password:arch" << std::endl;
            break;
        default:
            std::cout << "The program encountered some problems,the os has not been set,please try again,if you encounter issues again please contact the developers" << std::endl;           
            break;
    }
}

void set_tested_vm(const std::string &vm_name) {
    if (vm_name == "tails") {
        ship_env.os = TestedVM::tails;
    } else if (vm_name == "whonix") {
        ship_env.os = TestedVM::whonix;
    } else if (vm_name == "debian") {
        ship_env.os = TestedVM::debian;
    } else if (vm_name == "ubuntu") {
        ship_env.os = TestedVM::ubuntu;
    } else if (vm_name == "arch") {
        ship_env.os = TestedVM::arch;
    } else if (vm_name == "gentoo") {
        ship_env.os = TestedVM::gentoo;
    } else if (vm_name == "fedora") {
        ship_env.os = TestedVM::fedora;
    } else if (vm_name == "alpine") {
        ship_env.os = TestedVM::alpine;
    } else if (vm_name == "centos") {
        ship_env.os = TestedVM::centos;
    } else if (vm_name == "freebsd") {
        ship_env.os = TestedVM::freebsd;
    } else if (vm_name == "openbsd") {
        ship_env.os = TestedVM::openbsd;
    } else if (vm_name == "netbsd") {
        ship_env.os = TestedVM::netbsd;
    } else if (vm_name == "dragonflybsd") {
        ship_env.os = TestedVM::dragonflybsd;
    } else if (vm_name == "windows") {
        ship_env.os = TestedVM::windows;
    } else {
        std::cout << "The specified vm is not available as a tested and configured vm" << std::endl;
        return;
    }

    ship_env.source = get_tested_vm_link(vm_name);
}

void get_tested_vm() {
    while (true) {
        std::cout << "Please specify a vm from our tested and configured vms(use help to get the list of the available configured vms): ";
        std::getline(std::cin, ship_env.source);

        if (ship_env.source == "help") {
            print_available_tested_vms();
        } else {
            set_tested_vm(ship_env.source);
            if (ship_env.os != TestedVM::UNKNOWN) {
                break;
            }
        }
    }
}

void create_disk_image() {
    ship_env.disk_image_path = ship_lib_path + "images/disk-images/" + ship_env.name + generate_random_number(5) + ".qcow2";
    std::ifstream check_file(ship_env.disk_image_path);
    if (!check_file.good()) {
        std::cout << "Creating disk image at: " << ship_env.disk_image_path << std::endl;

        std::string create_disk_cmd = "qemu-img create -f qcow2 -o preallocation=metadata,cluster_size=512K " + ship_env.disk_image_path + " 2T";
        system_exec(create_disk_cmd);
    }
    check_file.close();
}

std::string get_disk_image_path() {
    std::string get_vm_disk_image_cmd = "virsh domblklist " + ship_env.name + " --details | grep vda | awk '{print $4}'";
    return trim_trailing_whitespaces(exec(get_vm_disk_image_cmd));
}

void convert_disk_image(const std::string &source_image, const std::string &dest_image, const std::string &options) {
    std::string convert_cmd = "sudo qemu-img convert -f qcow2 -O qcow2 " + options + " '" + source_image + "' '" + dest_image + "'";
    system_exec(convert_cmd);
}

void create_compact_disk_image() {
    std::string original_image_path = get_disk_image_path();
    std::string compact_image_path;

    while(true) {
        compact_image_path = ship_lib_path + "images/disk-images/" + ship_env.name + generate_random_number(5) + ".qcow2"; 

        std::ifstream check_file(compact_image_path);
        if (!check_file.good()) {
            break;
        }
        check_file.close();
    }


    std::cout << "Creating compact disk image at: " << compact_image_path << std::endl;
    convert_disk_image(original_image_path, compact_image_path, "-o preallocation=metadata,cluster_size=512K");
    std::cout << "Successfully created compact disk image: " << compact_image_path << std::endl; 

    std::cout << "Deleting the original disk image: " << original_image_path << std::endl;
    std::string delete_original_image_cmd = "rm " + original_image_path;
    system_exec(delete_original_image_cmd); 

    std::cout << "Making the vm use the compact disk image" << std::endl;  
    shutdown_vm();

    std::string detach_original_disk_cmd = "virsh detach-disk " + ship_env.name + " vda --persistent";
    system_exec(detach_original_disk_cmd); 

    std::string attach_compact_disk_cmd = "virsh attach-disk " + ship_env.name + " " + compact_image_path + " vda --driver qemu --subdriver qcow2 --persistent";
    system_exec(attach_compact_disk_cmd);

    std::cout << "Successfully replaced original disk image with compact version." << std::endl;
}

void generate_vm_name() {
    int next_vm_number = get_next_available_vm_number();
    ship_env.name = "vm" + std::to_string(next_vm_number);
}

void set_memory_limit() {
    if (ship_env.memory_limit.empty()) {
        int max_memory = stoi(exec("free -m | awk '/^Mem:/{print $2}'"));
        ship_env.memory_limit = std::to_string(max_memory);
    }
}

void set_cpu_limit() {
    if (ship_env.cpu_limit.empty()) {
        int max_cpus = stoi(exec("nproc"));
        ship_env.cpu_limit = std::to_string(max_cpus);
    }
}

void configure_vm() {
    std::cout << "Do you want do intial configuration for the proper working of  " << ship_env.name << " (y/n): ";
    std::string confirm;
    std::getline(std::cin, confirm);

    if (confirm == "y" || confirm == "Y") {
        std::cout << "Starting VM " << ship_env.name << "...\n";
    } else {
        std::cout << "VM has not been configured as indented but the vm has been made correctly and can be configured by yourself" << "'.\n";
        return;
    }
    start_vm();

    switch(ship_env.os) {
        case TestedVM::tails:
            return;
        case TestedVM::whonix:
            return;
        case TestedVM::debian:
            boost::property_tree::ini_parser::read_ini(find_settings_file(), pt);

            pt.put("system_exec.command_1", "root");

            boost::property_tree::ini_parser::write_ini(find_settings_file(), pt);

            run_startup_commands();
            return;
        case TestedVM::ubuntu:
            return;
        case TestedVM::arch:
            boost::property_tree::ini_parser::read_ini(find_settings_file(), pt);

            pt.put("credentials.hostname", "archlinux");
            pt.put("credentials.username", "arch");
            pt.put("credentials.password", "arch");

            pt.put("system.package_manager", "pacman");

            pt.put("xpra.enabled", true);

            pt.put("system_exec.command_1", "arch");
            pt.put("system_exec.command_2", "arch");
            pt.put("system_exec.command_3", "export display=:100");

            boost::property_tree::ini_parser::write_ini(find_settings_file(), pt);

            run_startup_commands();
            return;

        case TestedVM::gentoo:
            return;
        case TestedVM::fedora:
            return;
        case TestedVM::alpine:
            return;
        case TestedVM::centos:
            return;
        case TestedVM::freebsd:
            return;
        case TestedVM::openbsd:
            return;
        case TestedVM::netbsd:
            return;
        case TestedVM::dragonflybsd:
            return;
        case TestedVM::windows:
            return;
        default:
            return;
    }
}

void system_command_vm(const std::string& command) {
    std::string run_command_cmd = "tmux send-keys -t " + ship_env.name + " '" + command + "' C-m";
    system_exec(run_command_cmd);
}

bool exec_command_vm(const std::string& command) {

    std::string start_marker = "echo marker_" + std::to_string(rand());
    system_command_vm(start_marker);

    system_command_vm(command);

    std::string end_marker = "echo marker_" + std::to_string(rand());
    system_command_vm(end_marker); 

    std::string capture_cmd = "tmux capture-pane -t " + ship_env.name + " -pS - | tail -n 2 | head -n 1";

    while (true) {
        std::string output = exec(capture_cmd);
        if (output.find(end_marker) != std::string::npos) {
            break;
        }
    }

    capture_cmd = "tmux capture-pane -t " + ship_env.name + " -pS - | tac | grep -m1 -B " + std::to_string(INT_MAX) + " " + start_marker + " | tac | tail -n +3 | sed '/^\\s*$/d' | head -n -3";
    std::string output = exec(capture_cmd);

    if (ship_env.action == ShipAction::EXEC) {
        std::cout << output;
    }

    if (ship_env.action != ShipAction::EXEC && output.find_first_not_of(' ') != std::string::npos) {
        return true;
    }

    return false;
}

bool check_vm_command_exists(const std::string& command) {
    std::string check_command_exists_cmd = command + " --version > /dev/null 2>&1 && echo 0";
    bool result = exec_command_vm(check_command_exists_cmd);
    return result;
} 

void find_vm_package_manager() {
    try {
        boost::property_tree::ini_parser::read_ini(find_settings_file(), pt);

        std::cout << "Checking for package manager in the VM config" << std::endl;
        ship_env.package_manager_name = pt.get<std::string>("system.package_manager");
        std::cout << "Found package manager in the VM config" << std::endl;

    } catch(const boost::property_tree::ptree_error& e) {
        std::cout << "Package manager was not found in the config, trying to find the package manager manually. This might take a few moments..." << std::endl;

        for (const auto& package_manager : package_managers) {
            std::string package_manager_name = package_manager.first;
            if (check_vm_command_exists(package_manager_name)) {
                ship_env.package_manager_name = package_manager.first;             
                std::cout << "Package manager found as " << ship_env.package_manager_name << " for container " << ship_env.name << std::endl;
                return;
            }
        }
    }
}

void send_vm_file() {
    std::string get_vm_disk_image_cmd = "virsh domblklist " + ship_env.name + " --details | awk '{print $4}'";
    std::string disk_images = exec(get_vm_disk_image_cmd);

    std::vector<std::string> disk_image_paths = split_string_by_line(disk_images);

    std::string xml_file_path = "/tmp/" + ship_env.name + ".xml";
    std::string get_xml_file_cmd = "virsh dumpxml " + ship_env.name + " > " + xml_file_path;
    exec(get_xml_file_cmd); 

    std::string config_file = find_settings_file();

    std::string tar_file = "/tmp/" + ship_env.name + ".tar.gz";

    std::string create_tar_cmd = "tar -czf " + tar_file;
    for (const auto& path : disk_image_paths) {
        create_tar_cmd += " \"" + path + "\"";
    }
    create_tar_cmd += " \"" + xml_file_path + "\" \"" + config_file + "\"";

    system_exec(create_tar_cmd);

    std::string send_vm_cmd = "croc send " + tar_file;
    std::cout << exec(send_vm_cmd) << std::endl;
}

void receive_vm_file() {
    std::string code;
    std::cout << "Please type the secret code: ";
    std::getline(std::cin, code);

    std::string set_croc_secret_cmd = "export CROC_SECRET=" + code;
    std::cout << exec(set_croc_secret_cmd) << std::endl; 
    
    std::string receive_vm_cmd = "croc recv -o /tmp/";
    std::cout << exec(receive_vm_cmd) << std::endl;

    std::string find_tar_cmd = "find /tmp/ -type f -name '*.tar.gz' -exec ls -t1 {} + | head -1";
    std::string tar_file = exec(find_tar_cmd); 

    std::string extract_dir = "/tmp/" + ship_env.name + "/";
    std::string create_dir_cmd = "mkdir -p " + extract_dir;
    system_exec(create_dir_cmd); 

    std::string extract_tar_cmd = "tar -xzf " + tar_file + " -C " + extract_dir; 
    system_exec(extract_tar_cmd);
 
    std::string get_xml_file_cmd = "ls " + extract_dir + " | grep '\\.xml$' | head -n 1";
    std::string xml_file = exec(get_xml_file_cmd);

    std::string list_iso_files_cmd = "ls " + extract_dir + " | grep '\\.iso$'";
    std::string raw_iso_files_output = exec(list_iso_files_cmd);
    std::vector<std::string> parsed_iso_files = split_string_by_line(raw_iso_files_output);

    for (const auto& file : parsed_iso_files) {
        move_file(extract_dir + file, ship_lib_path + "images/iso-images/"); 
    }

    std::string list_qcow_files_cmd = "ls " + extract_dir + " | grep '\\.qcow2?$'";
    std::string raw_qcow_files_output = exec(list_qcow_files_cmd);
    std::vector<std::string> parsed_qcow_files = split_string_by_line(raw_qcow_files_output);

    for (const auto& file : parsed_qcow_files) {
        move_file(extract_dir + file, ship_lib_path + "images/disk-images/"); 
    } 

    std::string list_ini_files_cmd = "ls " + extract_dir + " | grep '\\.ini$'";
    std::string raw_ini_files_output = exec(list_ini_files_cmd);
    std::vector<std::string> parsed_ini_files = split_string_by_line(raw_ini_files_output);

    for (const auto& file : parsed_ini_files) {
        move_file(extract_dir + file, ship_lib_path + "settings/vm"); 
    }
    create_vm();
}

void exec_action_for_vm() {
    std::string group = "libvirt";
    add_user_to_group(group);
    setenv("LIBVIRT_DEFAULT_URI", "qemu:///system", 1);

    switch(ship_env.action) {
        case ShipAction::CREATE:
            create_vm();
            break;
        case ShipAction::START:
            start_vm();
            break;
        case ShipAction::RESTART:
            restart_vm();
            break;
        case ShipAction::DELETE:
            delete_vm();
            break;
        case ShipAction::VIEW:
            view_vm();
            break;
        case ShipAction::LIST:
            std::cout << list_vm();
            break;
        case ShipAction::PAUSE:
            pause_vm();
            break;
        case ShipAction::RESUME:
            resume_vm();
            break;
        case ShipAction::SAVE:
            save_vm();
            break;
        case ShipAction::SHUTDOWN:
            shutdown_vm();
            break;
        case ShipAction::EXEC:
            exec_command_vm(ship_env.command);
            break;
        case ShipAction::PACKAGE_DOWNLOAD:
        case ShipAction::PACKAGE_SEARCH:
        case ShipAction::PACKAGE_REMOVE:
            exec_package_manager_operations();
            break;
        case ShipAction::RECEIVE:
            receive_vm_file();
            break;
        case ShipAction::SEND:
            send_vm_file();
            break;
        case ShipAction::OPTIMIZE:
            create_compact_disk_image();
            break;
        default: 
            std::cout << "Invalid action for VM.\n";
            break;
    }
}

