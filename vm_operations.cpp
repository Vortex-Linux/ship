#include "ship.h"

void pass_password_to_tmux() {
    string capture_tmux_last_line_cmd = "tmux capture-pane -p -S -1 -t " + ship_env.name + " | tail -n 1";

    sleep(1);

    while (exec((capture_tmux_last_line_cmd.c_str())).find("password for") != string::npos) {
        cout << "Please provide your root password: ";     
        string root_password;     
        getline(cin, root_password); 
 
        ship_env.command = root_password;
        system_command_vm();

        sleep(1);
    }
}

void wait_for_vm_ready() {
    while(true) {
        string capture_tmux_last_line_cmd = "tmux capture-pane -p -S -1 -t " + ship_env.name + " | tail -n 1";
        string output = exec(capture_tmux_last_line_cmd.c_str());

        output = trim_trailing_whitespaces(output);

        if (!output.empty()) {
            break;
        }
        sleep(1);
    }
}

void run_startup_commands() {
    boost::property_tree::ini_parser::read_ini(find_settings_file(), pt);

    bool system_commands_left = true;
    bool exec_commands_left = true;
    int current_command_number = 1;

    while(system_commands_left || exec_commands_left) {
        try {
            ship_env.command = pt.get<std::string>("system.command_" + std::to_string(current_command_number));
            system_command_vm();
        } catch(const boost::property_tree::ptree_bad_path&) {
            system_commands_left = false;
        }

        sleep(1);

        try {
            ship_env.command = pt.get<std::string>("exec.command_" + std::to_string(current_command_number));
            exec_command_vm();
        } catch(const boost::property_tree::ptree_bad_path&) {
            exec_commands_left = false;
        } 

        sleep(1);

        current_command_number += 1;
    }
}

void start_vm() {
    string load_saved_cmd = "sudo virsh snapshot-revert --current " + ship_env.name;
    exec(load_saved_cmd.c_str());

    string state = get_vm_state(ship_env.name);

    if (state.find("running") == string::npos) {
          string start_cmd = "sudo virsh start " + ship_env.name;
          exec(start_cmd.c_str());
    }

    cout << "VM " << ship_env.name << " started successfully.\n";

    string create_tmux_session_cmd = "tmux new -d -s" + ship_env.name;
    system(create_tmux_session_cmd.c_str());

    ship_env.command = "sudo virsh console " + ship_env.name;
    system_command_vm();

    pass_password_to_tmux();

    wait_for_vm_ready();
    
    run_startup_commands();
}

void view_vm_console() {
    string view_cmd = "tmux attach-session -t " + ship_env.name;      
    system(view_cmd.c_str());
}

void view_vm_gui() {
    string view_cmd = "sudo virt-viewer " + ship_env.name; 
    system(view_cmd.c_str());
}

void view_vm() {
    cout << "Do you want a full GUI of the VM(By default the view action will show only a terminal of the VM) ? (y/n): ";     
    string confirm;     
    getline(cin, confirm); 

    if (confirm != "y" && confirm != "Y") {
        view_vm_console();
    }else {
        view_vm_gui();
    }   
}

void pause_vm() {
    string view_cmd = "sudo virsh suspend " + ship_env.name; 
    system(view_cmd.c_str());
}

void resume_vm() {
    string view_cmd = "sudo virsh resume " + ship_env.name; 
    system(view_cmd.c_str());
}

void delete_old_snapshots() {
    string list_snapshot_cmd = "sudo virsh snapshot-list --name " + ship_env.name;
    string snapshot_list = exec(list_snapshot_cmd.c_str());
    vector<string> snapshots = split_string_by_line(snapshot_list);

    reverse(snapshots.begin(), snapshots.end());

    string latest_snapshot = snapshots.front();

    for (const auto& snapshot : snapshots) {
        if (snapshot != latest_snapshot) {
            cout << "Deleting old snapshot: " << snapshot << endl;
            string delete_cmd = "sudo virsh snapshot-delete " + ship_env.name + " " + snapshot;
            system(delete_cmd.c_str());
        }
    }

    cout << "Kept latest snapshot: " << latest_snapshot << endl;
}

void save_vm() {
    string save_cmd = "sudo virsh snapshot-create --atomic " + ship_env.name; 
    system(save_cmd.c_str());

    delete_old_snapshots();
}

void shutdown_vm() {
    string shutdown_cmd = "sudo virsh shutdown " + ship_env.name; 
    system(shutdown_cmd.c_str()); 
    string state = get_vm_state(ship_env.name);

    if (state.find("running") != string::npos || state.find("paused") != string::npos) {
        cout << "The VM isnt responding do you want to forcefully shutdown the vm ? (y/n): ";
        string confirm;
        getline(cin, confirm);

        if (confirm != "y" && confirm != "Y") {
            cout << "VM force shutdown proccess cancelled for " << ship_env.name << "'.\n";
            return;
        }

        cout << "Forcefully shutting down " << ship_env.name << ".\n";
        string shutdown_cmd = "sudo virsh destroy " + ship_env.name + "\n";
        system(shutdown_cmd.c_str());
    }
    cout << "Successfully shutdown " << ship_env.name; 
}

void delete_vm() {
    string state = get_vm_state(ship_env.name);
    if (state.find("running") != string::npos || state.find("paused") != string::npos) {
        cout << "Forcefully shutting down VM " << ship_env.name << " before deletion.\n";
        string shutdown_cmd = "sudo virsh destroy " + ship_env.name;
        system(shutdown_cmd.c_str());
    }

    string undefine_cmd = "sudo virsh undefine --nvram " + ship_env.name;
    exec(undefine_cmd.c_str());
    cout << "VM " << ship_env.name << " deleted successfully.\n";
}

void create_vm() {
    
    bool custom_vm = true;

    if (ship_env.name.empty()) {
        generate_vm_name();
    }

    cout << "Creating new virtual machine for ship with name " + ship_env.name + "\n";

    if (ship_env.action == ShipAction::RECEIVE) {
        cout << "Please specify the name of this VM(leaving this blank will set the name to the name given by the sender which is " << ship_env.name << ")";
        string name_given;
        getline(cin, name_given);
        if(!name_given.empty()) {
            ship_env.name = name_given; 
        }
    }

    if (ship_env.source.empty() && ship_env.source_local.empty()) {
        custom_vm = false;
        get_tested_vm();
    }

    if (ship_env.source_local.empty() && ship_env.source.empty()) {
        cout << "Ship failed to create a VM because no valid source was specified.\n";
        exit(0);
    }

    get_iso_source();

    ship_env.source_local = trim_trailing_whitespaces(ship_env.source_local);

    if (ship_env.source_local.find_last_of(".") != string::npos) {
        string extension = ship_env.source_local.substr(ship_env.source_local.find_last_of("."));

        if (extension == ".iso") {
            ship_env.iso_path = get_absolute_path(ship_env.source_local);
            create_disk_image();
        }else if (extension == ".qcow2" || extension == ".qcow") {
            ship_env.disk_image_path = get_absolute_path(ship_env.source_local);
        }
    }

    set_memory_limit();
    set_cpu_limit();

    string xml_filename = generate_vm_xml();
    define_vm(xml_filename);

    if (!custom_vm) {
        configure_vm();
    } else {
        start_vm_with_confirmation_prompt();
    }

}

string generate_vm_xml() {
    ostringstream vm_xml;
    vm_xml << R"(
<domain type='kvm'>
  <name>)" << ship_env.name << R"(</name>
  <memory unit='MiB'>)" << ship_env.memory_limit << R"(</memory>
  <vcpu placement='static'>)" << ship_env.cpu_limit << R"(</vcpu>
  <os>
    <type arch='x86_64' machine='pc-i440fx-2.9'>hvm</type>
    <boot dev='hd'/>
    <boot dev='cdrom'/>
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
    <console type='pty'>
      <target type='virtio'/>
    </console>
    <sound model='ich9'/>
    <video>
      <model type='qxl' vram='65536' heads='1'/>
    </video>
    <memballoon model='virtio'/>
  </devices>
</domain>
)";

    string xml_filename = "/tmp/" + ship_env.name + ".xml";
    ofstream xml_file(xml_filename);
    xml_file << vm_xml.str();
    xml_file.close();

    ship_env.command = "touch " + get_absolute_path("./settings/vm-settings") + "/" + ship_env.name + ".ini";
    system(ship_env.command.c_str());

    return xml_filename;
}

void define_vm(const string& xml_filename) {
    string define_cmd = "sudo virsh define " + xml_filename;
    exec(define_cmd.c_str());

    cout << "New VM configuration defined.\n";
}

void start_vm_with_confirmation_prompt() {
    cout << "Do you want to start the VM " << ship_env.name << " right now? (y/n): ";
    string confirm;
    getline(cin, confirm);

    if (confirm == "y" || confirm == "Y") {
        cout << "Starting VM " << ship_env.name << "...\n";
    } else {
        cout << "VM has not been started. You can start it later with 'ship start " << ship_env.name << "'.\n";
        return;
    }
    start_vm();
}

void get_iso_source() {
    if(!ship_env.source.empty()) {
        cout << "Downloading iso to images" << "\n";
        string download_cmd = "aria2c --dir images/iso-images " + ship_env.source;
        system(download_cmd.c_str());
        cout << "Finding the path to the downloaded iso image" << "\n";
        string find_latest_image_cmd = "find images/iso-images  -type f -exec ls -t1 {} + | head -1";
        ship_env.source_local = exec(find_latest_image_cmd.c_str());
        cout << "Found the path as " << ship_env.source_local << "\n";
    }
}

void get_tested_vm() {
    while (true) {
        cout << "Please specify a vm from our tested and configured vms(use help to get the list of the available configured vms): ";
        getline(cin, ship_env.source);
        if (ship_env.source == "help") {
            cout << "The available tested and configured vms are: " << endl;
            cout << "tails" << endl;
            cout << "whonix" << endl;
            cout << "debian" << endl;
            cout << "ubuntu" << endl;
            cout << "arch" << endl;
            cout << "gentoo" << endl;
            cout << "fedora" << endl;
            cout << "centos" << endl;
            cout << "nix" << endl;
            cout << "alpine" << endl;
            cout << "Void" << endl;
            cout << "freebsd" << endl;
            cout << "openbsd" << endl;
            cout << "netbsd" << endl;
            cout << "dragonflybsd" << endl;
            cout << "windows" << endl;
            cout << "osx" << endl;
        }else {
            if (strcmp(ship_env.source.c_str(), "tails") == 0) {
                ship_env.os = TestedVM::tails; 
                ship_env.source = "https://download.tails.net/tails/stable/tails-amd64-6.4/tails-amd64-6.4.img";
            }else if (strcmp(ship_env.source.c_str(), "whonix") == 0) {
                ship_env.os = TestedVM::whonix; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "debian") == 0) {
                ship_env.os = TestedVM::debian; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "ubuntu") == 0) {
                ship_env.os = TestedVM::ubuntu; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "arch") == 0) {
                ship_env.os = TestedVM::arch; 
                ship_env.source = "https://geo.mirror.pkgbuild.com/images/latest/Arch-Linux-x86_64-basic.qcow2";
            }else if (strcmp(ship_env.source.c_str(), "gentoo") == 0) {
                ship_env.os = TestedVM::gentoo; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "fedora") == 0) {
                ship_env.os = TestedVM::fedora; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "nix") == 0) {
                ship_env.os = TestedVM::nix; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "alpine") == 0) {
                ship_env.os = TestedVM::alpine; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "centos") == 0) {
                ship_env.os = TestedVM::centos; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "Void") == 0) {
                ship_env.os = TestedVM::Void; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "freebsd") == 0) {
                ship_env.os = TestedVM::freebsd; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "openbsd") == 0) {
                ship_env.os = TestedVM::openbsd; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "netbsd") == 0) {
                ship_env.os = TestedVM::netbsd; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "dragonflybsd") == 0) {
                ship_env.os = TestedVM::dragonflybsd; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "windows") == 0) {
                ship_env.os = TestedVM::windows; 
                ship_env.source = "";
            }else if (strcmp(ship_env.source.c_str(), "osx") == 0) {
                ship_env.os = TestedVM::osx; 
                ship_env.source = "";
            }else {
                cout << "The specified vm is not available as a tested and configured vm" << endl;
                continue;
            }
            break;
        }
    }   
}

void create_disk_image() {
    ship_env.disk_image_path = get_absolute_path("./images/disk-images") + "/" + ship_env.name + ".qcow2";
    ifstream check_file(ship_env.disk_image_path);
    if (!check_file.good()) {
        cout << "Creating disk image at: " << ship_env.disk_image_path << endl;
        string create_disk_cmd = "qemu-img create -f qcow2 " + ship_env.disk_image_path + " 1G";
        system(create_disk_cmd.c_str());
    }
    check_file.close();
}

void generate_vm_name() {
    int next_vm_number = get_next_available_vm_number();
    ship_env.name = "vm" + to_string(next_vm_number);
}

void set_memory_limit() {
    if (ship_env.memory_limit.empty()) {
        int max_memory = stoi(exec("free -m | awk '/^Mem:/{print $2}'"));
        ship_env.memory_limit = to_string(max_memory);
    }
}

void set_cpu_limit() {
    if (ship_env.cpu_limit.empty()) {
        int max_cpus = stoi(exec("nproc"));
        ship_env.cpu_limit = to_string(max_cpus);
    }
}

void configure_vm() {
    cout << "Do you want do intial configuration for the proper working of  " << ship_env.name << " (y/n): ";
    string confirm;
    getline(cin, confirm);

    if (confirm == "y" || confirm == "Y") {
        cout << "Starting VM " << ship_env.name << "...\n";
    } else {
        cout << "VM has not been configured as indented but the vm has been made correctly and can be configured by yourself" << "'.\n";
        return;
    }
    start_vm();

    switch(ship_env.os) {
        case TestedVM::tails:
            return;
        case TestedVM::whonix:
            return;
        case TestedVM::debian:
            return;
        case TestedVM::ubuntu:
            return;
        case TestedVM::arch:
            boost::property_tree::ini_parser::read_ini(find_settings_file(), pt);

            pt.put("system.command_1", "arch");
            pt.put("system.command_2", "arch");

            boost::property_tree::ini_parser::write_ini(find_settings_file(), pt);

            run_startup_commands();

            ship_env.command = "sudo pacman-key --init";
            exec_command_vm();

            ship_env.command = "sudo pacman-key --populate-key archlinux";
            exec_command_vm();
            return;

        case TestedVM::gentoo:
            return;
        case TestedVM::fedora:
            return;
        case TestedVM::nix:
            return;
        case TestedVM::alpine:
            return;
        case TestedVM::centos:
            return;
        case TestedVM::Void:
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
        case TestedVM::osx:
            return;
        default:
            return;
    }
}

void start_vm(const string& name) {
    cout << "Starting VM " << name << "...\n";
    string start_cmd = "sudo virsh start " + name;
    exec(start_cmd.c_str());
}

void system_command_vm() {
    string run_cmd = "tmux send-keys -t " + ship_env.name + " '" + ship_env.command + "' C-m";
    system(run_cmd.c_str());
}

bool exec_command_vm() {
    string start_marker = "MARKER_" + to_string(rand());
    string start_marker_cmd = "tmux send-keys -t " + ship_env.name + " '" + start_marker + "' C-m";
    system(start_marker_cmd.c_str());

    system_command_vm();

    string end_marker = "MARKER_" + to_string(rand());
    string end_marker_cmd = "tmux send-keys -t " + ship_env.name + " 'echo " + end_marker + "' C-m";
    system(end_marker_cmd.c_str());

    string capture_cmd = "tmux capture-pane -t " + ship_env.name + " -pS - | tail -n 2 | head -n 1";

    while (true) {
        string output = exec(capture_cmd.c_str());
        if (output.find(end_marker) != string::npos) {
            break;
        }
    }

    capture_cmd = "tmux capture-pane -t " + ship_env.name + " -pS - | tac | grep -m1 -B " + to_string(INT_MAX) + " " + start_marker + " | tac | tail -n +3 | sed '/^\\s*$/d' | head -n -3";
    string output = exec(capture_cmd.c_str());

    if (ship_env.action == ShipAction::EXEC) {
        cout << output;
    }

    if (ship_env.action != ShipAction::EXEC && output.find_first_not_of(' ') != string::npos) {
        return true;
    }

    return false;
}

bool check_vm_command_exists() {
    ship_env.command += " --version > /dev/null 2>&1 && echo 0";
    bool result = exec_command_vm();
    return result;
} 

void find_vm_package_manager() {
    string parameters = ship_env.command;
    for (const auto& package_manager : package_managers) {
        ship_env.command = package_manager.first;
        if (check_vm_command_exists()) {
            ship_env.package_manager_name = package_manager.first;             
            cout << "Package manager found as " << ship_env.package_manager_name << " for container " << ship_env.name << "\n";
            ship_env.command = parameters;
            return;
        }
    }
}

void exec_action_for_vm() {
    switch(ship_env.action) {
        case ShipAction::CREATE:
            create_vm();
            break;
        case ShipAction::START:
            start_vm();
            break;
        case ShipAction::DELETE:
            delete_vm();
            break;
        case ShipAction::VIEW:
            view_vm();
            break;
        case ShipAction::LIST:
            cout << list_vm();
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
            exec_command_vm();
            break;
        case ShipAction::PACKAGE_DOWNLOAD:
        case ShipAction::PACKAGE_SEARCH:
        case ShipAction::PACKAGE_REMOVE:
            exec_package_manager_operations();
            break;
        case ShipAction::RECEIVE:
            receive_file();
            break;
        case ShipAction::SEND:
            send_file();
            break;
        default: 
            cout << "Invalid action for VM.\n";
            break;
    }
}

