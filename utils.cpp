#include "ship.h"

void show_help() {
    std::cout << "./ship [OPTIONS] COMMAND [ARGS...]" << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help                          Show this help message and exit" << std::endl;
    std::cout << "  --virtual-machine or --vm        Specify action is for VM" << std::endl;
    std::cout << "  --container or --ctr             Specify action is for container" << std::endl;
    std::cout << "  --name NAME                     Specifies the name of the container or VM an action should be executed on" << std::endl << std::endl;

    std::cout << "Commands:" << std::endl;
    std::cout << "  vm" << std::endl;
    std::cout << "    create" << std::endl;
    std::cout << "      --source URL                Set the source URL of the VM" << std::endl;
    std::cout << "      --source-local PATH         Set the local source path of the VM" << std::endl;
    std::cout << "      --cpus NUMBER               Set the CPU limit of the VM" << std::endl;
    std::cout << "      --memory or -mem SIZE       Set the memory limit of the VM" << std::endl;
    std::cout << "    start NAME                    Start the specified virtual machine" << std::endl;
  std::cout << "    delete NAME                   Delete the specified virtual machine" << std::endl;
    std::cout << "    list                          List all virtual machines" << std::endl;
    std::cout << "    view or enter NAME            Shows a console interface or a full GUI of the virtual machine" << std::endl;
    std::cout << "    pause NAME                    Pause the specified virtual machine" << std::endl;
    std::cout << "    resume NAME                   Resume the specified virtual machine if it's paused" << std::endl;
    std::cout << "    save NAME                     Take a snapshot of the specified virtual machine" << std::endl;
    std::cout << "    shutdown NAME                 Shut down the specified virtual machine" << std::endl;
    std::cout << "    exec NAME                     Execute the given command in the console of the specified virtual machine" << std::endl;
    std::cout << "      --command COMMAND           Set the command to be executed" << std::endl;
    std::cout << "    package_download or download_packages NAME" << std::endl;
    std::cout << "                                  Download the specified package using the package manager of the specified virtual machine" << std::endl;
    std::cout << "      --package PACKAGE           Set the package to be downloaded" << std::endl;
    std::cout << "    package_remove or remove_packages NAME" << std::endl;
    std::cout << "                                  Remove the specified package using the package manager of the specified virtual machine" << std::endl;
    std::cout << "      --package PACKAGE           Set the package to be removed" << std::endl;
    std::cout << "    package_search or search_packages NAME" << std::endl;
    std::cout << "                                  Search for the specified package using the package manager of the specified virtual machine" << std::endl;
    std::cout << "      --package PACKAGE           Set the package to be searched" << std::endl;
    std::cout << "    send NAME                     Share the specified virtual machine to an end user (protected by a secret code)" << std::endl;
    std::cout << "    receive                       Receive the virtual machine shared by an end user (protected by a secret code)" << std::endl << std::endl;

    std::cout << "  container" << std::endl;
    std::cout << "    create NAME                   Create a new container with the specified name" << std::endl;
    std::cout << "    delete NAME                   Delete the specified container" << std::endl;
    std::cout << "    view or enter NAME            Show a console interface" << std::endl;
    std::cout << "    upgrade NAME                  Upgrade the specified container" << std::endl;
    std::cout << "    list                          List all containers" << std::endl;
    std::cout << "    stop NAME                     Stop the specified container" << std::endl;
    std::cout << "    exec NAME                     Execute the given command in the console of the specified container" << std::endl;
    std::cout << "      --command COMMAND           Set the command to be executed" << std::endl;
    std::cout << "    package_download or download_packages NAME" << std::endl;
    std::cout << "                                  Download the specified package using the package manager of the specified container" << std::endl;
    std::cout << "      --package PACKAGE           Set the package to be downloaded" << std::endl;
    std::cout << "    package_remove or remove_packages NAME" << std::endl;
    std::cout << "                                  Remove the specified package using the package manager of the specified container" << std::endl;
    std::cout << "      --package PACKAGE           Set the package to be removed" << std::endl;
    std::cout << "    package_search or search_packages NAME" << std::endl;
    std::cout << "                                  Search for the specified package using the package manager of the specified container" << std::endl;
    std::cout << "      --package PACKAGE           Set the package to be searched" << std::endl;
    std::cout << "    send NAME                     Share the specified container to an end user (protected by a secret code)" << std::endl;
    std::cout << "    receive                       Receive the container shared by an end user (protected by a secret code)" << std::endl;
}

std::string trim_trailing_whitespaces(const std::string& str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == std::string::npos) {
        return ""; 
    }
    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, (last - first + 1));
}

void system_exec(const std::string& cmd) {
    int return_code = system(cmd.c_str());

    if (return_code != 0) {
        std::cerr << "Failed to execute command: " << cmd << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

std::string exec(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, int (*)(FILE*)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string list_vm() {
    std::string cmd = "virsh list --all";
    std::string result = exec(cmd);
    return result;
}

std::string list_container() {
    std::string cmd = "distrobox list";
    std::string result = exec(cmd);
    return result;
}

std::string get_vm_state(const std::string &vm_name) {
    std::string cmd = "virsh domstate " + vm_name;
    return exec(cmd);
}

std::vector<int> extract_numbers_with_prefix(const std::string& result,const std::string& prefix) {
    std::vector<int> numbers;
    std::istringstream stream(result);
    std::string line;
    while (getline(stream, line)) {
        size_t pos = line.find(prefix);
        if (pos != std::string::npos) {

            size_t start_pos = pos + prefix.length();
            std::string num_str = line.substr(start_pos);
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

int get_next_available_number_in_command_output(const std::string& command,const std::string& prefix) {
    std::string result = exec(command);
    std::vector<int> numbers = extract_numbers_with_prefix(result,prefix);

    if (numbers.empty()) {
        return 1;     
    }

    int max_number = *max_element(numbers.begin(), numbers.end());
    return max_number + 1;
}

int get_next_available_vm_number(){
    std::string command = "virsh list --all | grep vm";
    std::string prefix = "vm";
    int next_vm_number = get_next_available_number_in_command_output(command,prefix);
    return next_vm_number;
}

int get_next_available_container_number(){
    std::string command = "distrobox list | grep container";
    std::string prefix = "container_";
    int next_container_number = get_next_available_number_in_command_output(command,prefix);
    return next_container_number;
}

    
std::string get_absolute_path(const std::string &relative_path) {
    char abs_path[PATH_MAX];
    if (realpath(relative_path.c_str(), abs_path) != NULL) {
        return std::string(abs_path);
    } else {
        std::cout << "The path " << relative_path << " not found";
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
        std::cout << "Package manager not found\n";
    }
}

std::string generate_mac_address() {
    std::ostringstream mac;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0x00, 0xFF);

    mac << std::hex << std::setfill('0');
    mac << "52:54:00"; 
    for (int i = 0; i < 3; ++i) {
        mac << ":" << std::setw(2) << dis(gen);
    }

    return mac.str();
}

std::vector<std::string> split_string_by_line(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;
    while (std::getline(stream, token)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string find_settings_file() {
    if (ship_env.mode==ShipMode::CONTAINER){
        return get_executable_dir() + "settings/container-settings/" + ship_env.name + ".ini";
    }else {
        return get_executable_dir() + "settings/vm-settings/" + ship_env.name + ".ini";
    }
}

std::string get_executable_dir() {
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        char* last_slash = strrchr(path, '/');
        if (last_slash) {
            last_slash[1] = '\0'; 
        }
        return std::string(path);
    }
    return std::string();
}


bool is_user_in_group(const std::string& group) {
    std::string command = "id -nG \"$USER\" | grep -qw \"" + group + "\"";
    return (system(command.c_str()) == 0);
}

void add_user_to_group(const std::string& group) {
    if (!is_user_in_group(group)) {
        std::cout << "User is not in the " << group << " group." << std::endl;
        std::cout << "Adding user to the " << group << " group..." << std::endl;
        std::string command = "sudo usermod -aG " + group + " $(whoami)";
        system_exec(command);
    } 
}
