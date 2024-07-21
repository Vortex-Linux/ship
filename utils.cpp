#include "ship.h"

void show_help() {
    cout << "./ship [OPTIONS] COMMAND [ARGS...]" << endl << endl;
    cout << "Options:" << endl;
    cout << "  --help                          Show this help message and exit" << endl;
    cout << "  --virtual-machine or -vm        Specify action is for VM" << endl;
    cout << "  --container or -ctr             Specify action is for container" << endl;
    cout << "  --name NAME                     Specifies the name of the container or VM an action should be executed on" << endl << endl;

    cout << "Commands:" << endl;
    cout << "  vm" << endl;
    cout << "    create" << endl;
    cout << "      --source URL                Set the source URL of the VM" << endl;
    cout << "      --source-local PATH         Set the local source path of the VM" << endl;
    cout << "      --cpus NUMBER               Set the CPU limit of the VM" << endl;
    cout << "      --memory or -mem SIZE       Set the memory limit of the VM" << endl;
    cout << "    start NAME                    Start the specified virtual machine" << endl;
    cout << "    delete NAME                   Delete the specified virtual machine" << endl;
    cout << "    list                          List all virtual machines" << endl;
    cout << "    view or enter NAME            Shows a console interface or a full GUI of the virtual machine" << endl;
    cout << "    pause NAME                    Pause the specified virtual machine" << endl;
    cout << "    resume NAME                   Resume the specified virtual machine if it's paused" << endl;
    cout << "    save NAME                     Take a snapshot of the specified virtual machine" << endl;
    cout << "    shutdown NAME                 Shut down the specified virtual machine" << endl;
    cout << "    exec NAME                     Execute the given command in the console of the specified virtual machine" << endl;
    cout << "      --command COMMAND           Set the command to be executed" << endl;
    cout << "    package_download or download_packages NAME" << endl;
    cout << "                                  Download the specified package using the package manager of the specified virtual machine" << endl;
    cout << "      --package PACKAGE           Set the package to be downloaded" << endl;
    cout << "    package_remove or remove_packages NAME" << endl;
    cout << "                                  Remove the specified package using the package manager of the specified virtual machine" << endl;
    cout << "      --package PACKAGE           Set the package to be removed" << endl;
    cout << "    package_search or search_packages NAME" << endl;
    cout << "                                  Search for the specified package using the package manager of the specified virtual machine" << endl;
    cout << "      --package PACKAGE           Set the package to be searched" << endl;
    cout << "    send NAME                     Share the specified virtual machine to an end user (protected by a secret code)" << endl;
    cout << "    receive                       Receive the virtual machine shared by an end user (protected by a secret code)" << endl << endl;

    cout << "  container" << endl;
    cout << "    create NAME                   Create a new container with the specified name" << endl;
    cout << "    delete NAME                   Delete the specified container" << endl;
    cout << "    view or enter NAME            Show a console interface" << endl;
    cout << "    upgrade NAME                  Upgrade the specified container" << endl;
    cout << "    list                          List all containers" << endl;
    cout << "    stop NAME                     Stop the specified container" << endl;
    cout << "    exec NAME                     Execute the given command in the console of the specified container" << endl;
    cout << "      --command COMMAND           Set the command to be executed" << endl;
    cout << "    package_download or download_packages NAME" << endl;
    cout << "                                  Download the specified package using the package manager of the specified container" << endl;
    cout << "      --package PACKAGE           Set the package to be downloaded" << endl;
    cout << "    package_remove or remove_packages NAME" << endl;
    cout << "                                  Remove the specified package using the package manager of the specified container" << endl;
    cout << "      --package PACKAGE           Set the package to be removed" << endl;
    cout << "    package_search or search_packages NAME" << endl;
    cout << "                                  Search for the specified package using the package manager of the specified container" << endl;
    cout << "      --package PACKAGE           Set the package to be searched" << endl;
    cout << "    send NAME                     Share the specified container to an end user (protected by a secret code)" << endl;
    cout << "    receive                       Receive the container shared by an end user (protected by a secret code)" << endl;
}

string trim_trailing_whitespaces(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) {
        return ""; 
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, int (*)(FILE*)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

string list_vm() {
    string cmd = "sudo virsh list --all";
    string result = exec(cmd.c_str());
    return result;
}

string list_container() {
    string cmd = "distrobox list";
    string result = exec(cmd.c_str());
    return result;
}

string get_vm_state(const string &vm_name) {
    string cmd = "sudo virsh domstate " + vm_name;
    return exec(cmd.c_str());
}

vector<int> extract_numbers_with_prefix(const string& result,const string& prefix) {
    vector<int> numbers;
    istringstream stream(result);
    string line;
    while (getline(stream, line)) {
        size_t pos = line.find(prefix);
        if (pos != string::npos) {
            string num_str = line.substr(pos + 2);
            try {
                int num = stoi(num_str);
                numbers.push_back(num);
            } catch (...) {
                // Ignore any invalid numbers
            }
        }
    }
    return numbers;
}

int get_next_available_number_in_command_output(const string& command,const string& prefix) {
    string result = exec(command.c_str());
    vector<int> numbers = extract_numbers_with_prefix(result,prefix);

    if (numbers.empty()) {
        return 1;     
    }

    int max_number = *max_element(numbers.begin(), numbers.end());
    return max_number + 1;
}

int get_next_available_vm_number(){
    string command = "sudo virsh list --all | grep vm";
    string prefix = "vm";
    int next_vm_number = get_next_available_number_in_command_output(command,prefix);
    return next_vm_number;
}

int get_next_available_container_number(){
    string command = "distrobox list | grep vm";
    string prefix = "container";
    int next_container_number = get_next_available_number_in_command_output(command,prefix);
    return next_container_number;
}

    
string get_absolute_path(const string &relative_path) {
    char abs_path[PATH_MAX];
    if (realpath(relative_path.c_str(), abs_path) != NULL) {
        return string(abs_path);
    } else {
        cout << "The path " << relative_path << " not found";
        exit(0);
    }
}

void exec_package_manager_operations() {
    if(ship_env.mode==ShipMode::CONTAINER) {
        find_container_package_manager();
    }else {
        find_vm_package_manager();
    }

    auto it = package_managers.find(ship_env.package_manager_name);

    if (it != package_managers.end()) {
        const PackageManagerCommands& commands = it->second;

        if (ship_env.action == ShipAction::PACKAGE_SEARCH) {
            ship_env.command = commands.search_command + " " + ship_env.packages + " " + ship_env.command;
        } else if (ship_env.action == ShipAction::PACKAGE_DOWNLOAD) {
            ship_env.command = commands.install_command + " " + ship_env.packages + " " + ship_env.command;
        } else if (ship_env.action == ShipAction::PACKAGE_REMOVE) {
            ship_env.command = commands.remove_command + " " + ship_env.packages + " " + ship_env.command;
        }

        ship_env.command = "yes | " + ship_env.command;

        if(ship_env.mode==ShipMode::CONTAINER) {
            exec_command_container();
        } else {
            ship_env.action = ShipAction::EXEC;
            exec_command_vm();
        }

    } else {
        cout << "Package manager not found\n";
    }
}

string generate_mac_address() {
    ostringstream mac;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0x00, 0xFF);

    mac << std::hex << std::setfill('0');
    mac << "52:54:00"; 
    for (int i = 0; i < 3; ++i) {
        mac << ":" << std::setw(2) << dis(gen);
    }

    return mac.str();
}

vector<string> split_string_by_line(const string& str) {
    vector<string> tokens;
    istringstream stream(str);
    string token;
    while (getline(stream, token)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

void send_file() {
    if(ship_env.mode==ShipMode::CONTAINER) {
        string image = "/tmp/" + ship_env.name;

        string make_container_image_cmd = "docker commit " + ship_env.name + " " + ship_env.name;
        cout << exec(make_container_image_cmd.c_str()) << endl;

        string make_container_tar_file_cmd = "docker save -o " + image + " " + ship_env.name;
        cout << exec(make_container_tar_file_cmd.c_str()) << endl;

        string send_container_image_cmd = "croc send " + image;
        cout << exec(send_container_image_cmd.c_str()) << endl;

        string image_cleanup_cmd = "docker rmi " + ship_env.name;
        cout << exec(image_cleanup_cmd.c_str()) << endl;

    }else {
        string get_vm_disk_image_cmd = "sudo virsh domblklist " + ship_env.name + " --details | awk '/source file/ {print $3}' | grep '.qcow2$'";
        string result = exec(get_vm_disk_image_cmd.c_str());
        string send_vm_cmd = "croc send " + get_absolute_path(result);
        cout << exec(send_vm_cmd.c_str()) << endl;
    } 
}

void receive_file() {
    string code;
    cout << "Please type the secret code: ";
    getline(cin, code);

    if(ship_env.mode==ShipMode::CONTAINER) {
        string set_croc_secret_cmd = "export CROC_SECRET=" + code;
        cout << exec(set_croc_secret_cmd.c_str()) << endl;

        string receive_container_cmd = "croc recv";
        cout << exec(receive_container_cmd.c_str()) << endl;

        string find_container_image_cmd = "ls -t | head -1";
        string image = exec(find_container_image_cmd.c_str());

        string load_container_image_cmd = "docker load -i " + image;
        cout << exec(load_container_image_cmd.c_str()) << endl;

        string create_container_cmd = "distrobox create --name " + image + " --image " + image;
        cout << exec(create_container_cmd.c_str()) << endl;

    }else {
        ship_env.source_local = get_absolute_path("images/disk-images/");

        string set_croc_secret_cmd = "export CROC_SECRET=" + code;
        cout << exec(set_croc_secret_cmd.c_str()) << endl;

        string receive_vm_cmd = "croc recv -o " + ship_env.source_local;
        cout << exec(receive_vm_cmd.c_str()) << endl;

        string find_vm_image_cmd = "find /images/disk-images/  -type f -exec ls -t1 {} + | head -1";
        string vm_image = exec(find_vm_image_cmd.c_str());

        size_t extension_starting_position = ship_env.source_local.find(".");
        string image_name = ship_env.source_local.substr(0, extension_starting_position);

        ship_env.name = image_name;

        create_vm();
  }
}

