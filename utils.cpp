#include "ship.h"

void show_help() {
    cout << "Usage: ship [OPTIONS] COMMAND [ARGS...]\n";
    cout << "Options:\n";
    cout << "  --help              Show this help message and exit\n";
    cout << "  -v, --verbose       Enable verbose output\n";
    cout << "Commands:\n";
    cout << "  create              Create a new virtual machine\n";
    cout << "    --name NAME           Set the name of the VM\n";
    cout << "    --source URL          Set the source URL of the VM\n";
    cout << "    --source-local PATH   Set the local source path of the VM\n";
    cout << "    --cpus NUMBER         Set the CPU limit of the VM\n";
    cout << "    --memory SIZE         Set the memory limit of the VM\n";
    cout << "  start NAME          Start the specified virtual machine\n";
    cout << "  delete NAME         Delete the specified virtual machine\n";
    cout << "  list                List all virtual machines\n";
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
    string prefix = "container";
    int next_vm_number = get_next_available_number_in_command_output(command,prefix);
    return next_vm_number;
}

int get_next_available_container_number(){
    string command = "distrobox list | grep vm";
    string prefix = "vm";
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