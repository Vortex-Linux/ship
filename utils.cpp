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

string get_vm_state(const string &vm_name) {
    string cmd = "sudo virsh domstate " + vm_name;
    return exec(cmd.c_str());
}

vector<int> extract_vm_numbers(const string& result) {
    vector<int> vm_numbers;
    istringstream stream(result);
    string line;
    while (getline(stream, line)) {
        size_t pos = line.find("vm");
        if (pos != string::npos) {
            string num_str = line.substr(pos + 2);
            try {
                int num = stoi(num_str);
                vm_numbers.push_back(num);
            } catch (...) {
                // Ignore any invalid numbers
            }
        }
    }
    return vm_numbers;
}

int get_next_available_vm_number() {
    string result = exec("sudo virsh list --all | grep vm");
    vector<int> vm_numbers = extract_vm_numbers(result);

    if (vm_numbers.empty()) {
        return 1;     }

    int max_vm_number = *max_element(vm_numbers.begin(), vm_numbers.end());
    return max_vm_number + 1;
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
