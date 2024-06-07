#include "ship.h"

void start_vm() {
    string start_cmd = "sudo virsh start " + name;
    exec(start_cmd.c_str());

    cout << "VM " << name << " started successfully.\n";

    cout << "Entering VM for like login proccess if its there as ship has no knowledge about the inside woorking of a particular os please type login details or anything to get ito where you can type commands and then use Ctrl+B and then D to exit\n";
    cout << "Press Enter to continue...\n";
    cin.get();

    string create_tmux_session_cmd = "tmux new -d -s" + name;
    system(create_tmux_session_cmd.c_str());

string run_console_cmd = "tmux send-keys -t " + name + " 'sudo virsh console " + name + "' C-m";
    system(run_console_cmd.c_str());


    string attach_console_session_cmd = "tmux attach-session -t " + name;
    system(attach_console_session_cmd.c_str());
}

void view_vm_console() {
    string view_cmd = "sudo virsh console " + name;      
    system(view_cmd.c_str());
}

void view_vm_gui() {
    string view_cmd = "sudo virt-viewer " + name; 
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
    string view_cmd = "sudo virsh suspend " + name; 
    system(view_cmd.c_str());
}

void resume_vm() {
    string view_cmd = "sudo virsh resume " + name; 
    system(view_cmd.c_str());
}

void shutdown_vm() {
    string shutdown_cmd = "sudo virsh shutdown " + name; 
    system(shutdown_cmd.c_str()); 
    string state = get_vm_state(name);

    if (state.find("running") != string::npos || state.find("paused") != string::npos) {
        cout << "The VM isnt responding do you want to forcefully shutdown the vm ? (y/n): ";
        string confirm;
        getline(cin, confirm);

        if (confirm != "y" && confirm != "Y") {
            cout << "VM force shutdown proccess cancelled for " << name << "'.\n";
            return;
        }

        cout << "Forcefully shutting down " << name << ".\n";
        string shutdown_cmd = "sudo virsh destroy " + name + ".\n";
        system(shutdown_cmd.c_str());
    }
    cout << "Successfully shutdown " << name; 
}

void delete_vm() {
    string state = get_vm_state(name);
    if (state.find("running") != string::npos || state.find("paused") != string::npos) {
        cout << "Forcefully shutting down VM " << name << " before deletion.\n";
        string shutdown_cmd = "sudo virsh destroy " + name;
        system(shutdown_cmd.c_str());
    }

    string undefine_cmd = "sudo virsh undefine " + name;
    exec(undefine_cmd.c_str());
    cout << "VM " << name << " deleted successfully.\n";
}

void create_vm() {
    cout << "Creating new virtual machine for ship\n";

    if (name.empty()) {
        int next_vm_number = get_next_available_vm_number();
        name = "vm" + to_string(next_vm_number);
        cout << "Please specify the name of this VM(leaving this blank will set the name to " << name << ")";
        string name_given;
        getline(cin, name_given);
        if(!name_given.empty()) {
            name = name_given; 
        }
    }

    if (source.empty() && source_local.empty()) {
        cout << "Please specify the source of this VM (leave this blank if you want to specify a local source): ";
        getline(cin, source);
    }

    if (source_local.empty() && source.empty()) {
        cout << "Please specify the local source of this VM: ";
        getline(cin, source_local);
        if (source_local.empty() && source.empty()) {
            cout << "Ship failed to create a VM, no source has been specified.\n";
            exit(0);
        }
    }

    if(!source.empty()) {
        cout << "Downloading iso to images(Please use ctrl+c after the download proccess is complete to come back to the main program)" << "\n";
        string download_cmd = "aria2c --dir images " + source;
        system(download_cmd.c_str());
        cout << "Finding the path to the downloaded iso image" << "\n";
        string find_latest_image_cmd = "find images  -type f -exec ls -t1 {} + | head -1";
        source_local = exec(find_latest_image_cmd.c_str());
        cout << "Found the path as " << source_local << "\n";
    }

    if (memory_limit.empty()) {
        cout << "Please specify the memory limit of this VM (leave blank for no limit): ";
        getline(cin, memory_limit);
    }
    if (memory_limit.empty()) {
        int max_memory = stoi(exec("free -m | awk '/^Mem:/{print $2}'"));
        memory_limit = to_string(max_memory);
    }

    if (cpu_limit.empty()) {
        cout << "Please specify the CPU limit of this VM (leave blank for no limit): ";
        getline(cin, cpu_limit);
    }
    if (cpu_limit.empty()) {
        int max_cpus = stoi(exec("nproc"));
        cpu_limit = to_string(max_cpus);
    }

    source_local = get_absolute_path(source_local);

    stringstream vm_xml;
    vm_xml << R"(
<domain type='kvm'>
  <name>)" << name << R"(</name>
  <memory unit='MiB'>)" << memory_limit << R"(</memory>
  <vcpu placement='static'>)" << cpu_limit << R"(</vcpu>
  <os>
    <type arch='x86_64' machine='pc-i440fx-2.9'>hvm</type>
    <boot dev='hd'/>
    <boot dev='cdrom'/>
  </os>
  <devices>
    <disk type='file' device='cdrom'>
      <driver name='qemu' type='raw'/>
      <source file=')" << source_local << R"('/>
      <target dev='hdc' bus='ide'/>
      <readonly/>
    </disk>
    <filesystem type='mount'>
      <source dir='/'/>
      <target dir='host_files'/>
    </filesystem>
    <interface type='network'>
      <source network='default'/>
    </interface>
    <input type='tablet' bus='usb'/>
    <input type='mouse' bus='usb'/>
    <input type='keyboard' bus='usb'/>
    <graphics type='vnc' port='-1' autoport='yes' listen='0.0.0.0'>
      <listen type='address' address='0.0.0.0'/>
    </graphics>
    <graphics type='spice' autoport='yes'/>
    <console type='pty'>
      <target type='virtio' port='0'/>
    </console>
    <controller type='usb' index='0'/>
    <controller type='scsi' index='0'/>
    <controller type='pci' index='0' model='pci-root'/>
  </devices>
</domain>
)";

    string xml_filename = "/tmp/" + name + ".xml";
    ofstream xml_file(xml_filename);
    xml_file << vm_xml.str();
    xml_file.close();

    string define_cmd = "sudo virsh define " + xml_filename;
    exec(define_cmd.c_str());

    cout << "New VM " << name << " was created successfully.\n";

    cout << "Do you want to start the VM right now " << name << "? (y/n): ";
    string confirm;
    getline(cin, confirm);

    if (confirm != "y" && confirm != "Y") {
        cout << "VM has not been started, you can start it at any time with 'ship start " << name << "'.\n";
        return;
    }

    start_vm();
}

bool exec_command_vm() {
    string start_marker = "MARKER_" + to_string(rand());
    string start_marker_cmd = "tmux send-keys -t " + name + " '" + start_marker + "' C-m";
    system(start_marker_cmd.c_str());

    string run_cmd = "tmux send-keys -t " + name + " '" + command + "' C-m";
    system(run_cmd.c_str());

    string end_marker = "MARKER_" + to_string(rand());
    string end_marker_cmd = "tmux send-keys -t " + name + " 'echo " + end_marker + "' C-m";
    system(end_marker_cmd.c_str());

    string capture_cmd = "tmux capture-pane -t " + name + " -pS - | tail -n 2 | head -n 1";

    while (true) {
        string output = exec(capture_cmd.c_str());
        if (output.find(end_marker) != string::npos) {
            break;
        }
    }

    capture_cmd = "tmux capture-pane -t " + name + " -pS - | tac | grep -m1 -B " + to_string(INT_MAX) + " " + start_marker + " | tac | tail -n +3 | sed '/^\\s*$/d' | head -n -3";
    string output = exec(capture_cmd.c_str());

    if (action == "exec") {
        cout << output;
    }

    if (action != "exec" && output.find_first_not_of(' ') != string::npos) {
        return true;
    }

    return false;
}

bool check_vm_command_exists() {
    command += " --version > /dev/null 2>&1 && echo 0";
    bool result = exec_command_vm();
    return result;
} 

void find_vm_package_manager() {
    string parameters = command;
    for (const auto& package_manager : package_managers) {
        command = package_manager.first;
        if (check_vm_command_exists()) {
            package_manager_name = package_manager.first;             
            cout << "Package manager found as " << package_manager_name << " for container " << name << "\n";
            command = parameters;
            return;
        }
    }
}

void exec_action_for_vm() {
    if (action == "create") {
        create_vm();
    } else if (action == "start") {
        start_vm();
    } else if (action == "delete") {
        delete_vm();
    } else if (action == "view") {
        view_vm();
    } else if (action == "list") {
        cout << list_vm();
    } else if (action == "pause") {
        pause_vm();
    } else if (action == "resume") {
        resume_vm();
    } else if (action == "shutdown") {
        shutdown_vm();
    } else if (action == "exec") {
        exec_command_vm();
    } else if (action == "package_download") {
        exec_package_manager_operations();
    } else if (action == "package_search") {
        exec_package_manager_operations();
    } else if (action == "package_remove") {
        exec_package_manager_operations();
    }
}

