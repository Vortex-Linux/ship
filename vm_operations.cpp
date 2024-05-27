#include "ship.h"

void start_vm() {
    string start_cmd = "sudo virsh start " + name;
    exec(start_cmd.c_str());
    cout << "VM " << name << " started successfully.\n";
}

void view_vm() {
    string view_cmd = "sudo virt-viewer " + name; 
    system(view_cmd.c_str());
}

void shutdown_vm() {
    string state = get_vm_state(name);
    string view_cmd = "sudo virsh shutdown " + name; 
    system(view_cmd.c_str());
}

void delete_vm() {
    string state = get_vm_state(name);
    if (state.find("running") != string::npos || state.find("paused") != string::npos) {
        cout << "Shutting down VM " << name << " before deletion.\n";
        string shutdown_cmd = "sudo virsh destroy " + name;
        exec(shutdown_cmd.c_str());
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

void process_operands(int argc, char *argv[]) {
    bool fetching_name = false;
    bool fetching_source = false;
    bool fetching_local_source = false;
    bool fetching_cpu_limit = false;
    bool fetching_memory_limit = false;
    int action_index = INT_MAX;

    for (int i = 1; i < argc; ++i) {
        if (fetching_name) {
            name = argv[i];
            fetching_name = false;
            continue;
        }
        if (fetching_source) {
            source = argv[i];
            source_local = "";
            fetching_source = false;
            continue;
        }
        if (fetching_local_source) {
            source_local = argv[i];
            source = "";
            fetching_local_source = false;
            continue;
        }
        if (fetching_cpu_limit) {
            cpu_limit = argv[i];
            fetching_cpu_limit = false;
            continue;
        }
        if (fetching_memory_limit) {
            memory_limit = argv[i];
            fetching_memory_limit = false;
            continue;
        }

        if (strcmp(argv[i], "--help") == 0) {
            show_help();
            return;
        }
        if (strcmp(argv[i], "create") == 0 && action == "") {
            creating_vm = true;
            action = "create";
            action_index = i;
            continue;
        }
        if (strcmp(argv[i], "--name") == 0 && action_index!=INT_MAX) {
            fetching_name = true;
            continue;
        }
        if (strcmp(argv[i], "--source") == 0 && action=="create") {
            fetching_source = true;
            continue;
        }
        if (strcmp(argv[i], "--source-local") == 0 && action=="create") {
            fetching_local_source = true;
            continue;
        }
        if (strcmp(argv[i], "--cpus") == 0 && action=="create") {
            fetching_cpu_limit = true;
            continue;
        }
        if ((strcmp(argv[i], "--memory") == 0 || strcmp(argv[i], "--mem") == 0) && action=="create") {
            fetching_memory_limit = true;
            continue;
        }
        if (strcmp(argv[i], "start") == 0 && action == "") {
            starting_vm = true;
            action = "start";
            action_index = i;
            continue;
        }
        if (strcmp(argv[i], "delete") == 0 && action == "") {
            deleting_vm = true;
            action = "delete";
            action_index = i;
            continue;
        }
        if (strcmp(argv[i], "list") == 0 && action == "") {
            listing_vm = true;
            action = "list";
            action_index = i;
            continue;
        }
        if (strcmp(argv[i], "view") == 0 && action == "") {
            viewing_vm = true;
            action = "view";
            action_index = i;
            continue;
        }

        if (i - 1 == action_index) {
            name = argv[i];
            continue;
        }

        if (action == "") {
            cout << "Ship found unknown operand " << argv[i] << "\n";
            cout << "For more information try ship --help\n";
        } else {
            cout << "Ship found unknown operand " << argv[i] << " for action " << action << "\n";
        }

        exit(1);
    }
}

void exec_action() {
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
    }
}

