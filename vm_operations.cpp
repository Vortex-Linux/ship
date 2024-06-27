#include "ship.h"

void start_vm() {
    string load_saved_cmd = "sudo virsh snapshot-revert --current " + name;
    exec(load_saved_cmd.c_str());

    string state = get_vm_state(name);

    if (state.find("running") == string::npos) {
          string start_cmd = "sudo virsh start " + name;
          exec(start_cmd.c_str());
    }

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
    string view_cmd = "tmux attach-session -t " + name;      
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

void delete_old_snapshots() {
    string list_snapshot_cmd = "sudo virsh snapshot-list --name " + name;
    string snapshot_list = exec(list_snapshot_cmd.c_str());
    vector<string> snapshots = split_string_by_line(snapshot_list);

    reverse(snapshots.begin(), snapshots.end());

    string latest_snapshot = snapshots.front();

    for (const auto& snapshot : snapshots) {
        if (snapshot != latest_snapshot) {
            cout << "Deleting old snapshot: " << snapshot << endl;
            string delete_cmd = "sudo virsh snapshot-delete " + name + " " + snapshot;
            system(delete_cmd.c_str());
        }
    }

    cout << "Kept latest snapshot: " << latest_snapshot << endl;
}

void save_vm() {
    string save_cmd = "sudo virsh snapshot-create --atomic " + name; 
    system(save_cmd.c_str());

    delete_old_snapshots();
}

void shutdown_vm() {
    cout << "Do you want to save the virtual machine before shutting it down: ? (y/n): ";     
    string confirm;     
    getline(cin, confirm); 

    if (confirm == "y" || confirm == "Y") {
        save_vm();
    }

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
        string shutdown_cmd = "sudo virsh destroy " + name + "\n";
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

    string undefine_cmd = "sudo virsh undefine --nvram " + name;
    exec(undefine_cmd.c_str());
    cout << "VM " << name << " deleted successfully.\n";
}

void create_vm() {
    if (name.empty()) {
        generate_vm_name();
    }

    cout << "Creating new virtual machine for ship with name " + name + "\n";

    if (action == "receive") {
        cout << "Please specify the name of this VM(leaving this blank will set the name to the name given by the sender which is " << name << ")";
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

    get_iso_source();

    if(!source_local.empty() && source.empty()) {
        if (source_local.find_last_of(".") != string::npos) {
            string extension = source_local.substr(source_local.find_last_of("."));
            if (extension == ".iso") {
                iso_path = get_absolute_path(source_local);
                create_disk_image();
            }else if (extension == ".qcow2" || extension == ".qcow") {
                disk_image_path = get_absolute_path(source_local);
            }
        }
    }

    set_memory_limit();
    set_cpu_limit();

    string xml_filename = generate_vm_xml();
    define_vm(xml_filename);

    start_vm_with_confirmation_prompt();
}

string generate_vm_xml() {
    ostringstream vm_xml;
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
      <source file=')" << disk_image_path << R"('/>
      <target dev='vda' bus='virtio'/>
    </disk>)";

    if (!iso_path.empty()) {
        vm_xml << R"(
    <disk type='file' device='cdrom'>
      <driver name='qemu' type='raw'/>
      <source file=')" << iso_path << R"('/>
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

    string xml_filename = "/tmp/" + name + ".xml";
    ofstream xml_file(xml_filename);
    xml_file << vm_xml.str();
    xml_file.close();

    return xml_filename;
}

void define_vm(const string& xml_filename) {
    string define_cmd = "sudo virsh define " + xml_filename;
    exec(define_cmd.c_str());

    cout << "New VM configuration defined.\n";
}

void start_vm_with_confirmation_prompt() {
    cout << "Do you want to start the VM " << name << " right now? (y/n): ";
    string confirm;
    getline(cin, confirm);

    if (confirm == "y" || confirm == "Y") {
        cout << "Starting VM " << name << "...\n";
    } else {
        cout << "VM has not been started. You can start it later with 'ship start " << name << "'.\n";
        return;
    }
    start_vm();
}

void get_iso_source() {
    if(!source.empty()) {
        cout << "Downloading iso to images(Please use ctrl+c after the download proccess is complete to come back to the main program)" << "\n";
        string download_cmd = "aria2c --dir images/iso-images " + source;
        system(download_cmd.c_str());
        cout << "Finding the path to the downloaded iso image" << "\n";
        string find_latest_image_cmd = "find images/iso-images  -type f -exec ls -t1 {} + | head -1";
        source_local = exec(find_latest_image_cmd.c_str());
        cout << "Found the path as " << source_local << "\n";
    }
}

void create_disk_image() {
    disk_image_path = get_absolute_path("./images/disk-images") + "/" + name + ".qcow2";
    ifstream check_file(disk_image_path);
    if (!check_file.good()) {
        cout << "Creating disk image at: " << disk_image_path << endl;
        string create_disk_cmd = "qemu-img create -f qcow2 " + disk_image_path + " 1P";
        system(create_disk_cmd.c_str());
    }
    check_file.close();
}

void generate_vm_name() {
    int next_vm_number = get_next_available_vm_number();
    name = "vm" + to_string(next_vm_number);
}

void set_memory_limit() {
    if (memory_limit.empty()) {
        int max_memory = stoi(exec("free -m | awk '/^Mem:/{print $2}'"));
        memory_limit = to_string(max_memory);
    }
}

void set_cpu_limit() {
    if (cpu_limit.empty()) {
        int max_cpus = stoi(exec("nproc"));
        cpu_limit = to_string(max_cpus);
    }
}

void start_vm(const string& name) {
    cout << "Starting VM " << name << "...\n";
    string start_cmd = "sudo virsh start " + name;
    exec(start_cmd.c_str());
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
    if (!package_manager_name.empty()) {
        
    }

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
    } else if (action == "save") {
        save_vm();
    } else if (action == "shutdown") {
        shutdown_vm();
    } else if (action == "exec") {
        exec_command_vm();
    } else if (action == "package_download" || action == "package_search" || action == "package_remove") {
        exec_package_manager_operations();
    } else if (action == "receive") {
        receive_file();
    } else if (action == "send") {
        send_file();
    }
}

