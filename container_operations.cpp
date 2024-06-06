#include "ship.h"

void stop_container() {
    string cmd = "distrobox stop " + name;
    string result = exec(cmd.c_str());
    system(cmd.c_str());

}

void delete_container() {
    string cmd = "distrobox rm " + name;
    system(cmd.c_str());
}

void create_container() {

    if (name.empty()) {
        int next_container_number = get_next_available_container_number();
        name = "container_" + to_string(next_container_number);
        cout << "Please specify the name of this container(leaving this blank will set the name to " << name << ")";
        string name_given;
        getline(cin, name_given);
        if(!name_given.empty()) {
            name = name_given; 
        }
    }

    if (image.empty()) {
        cout << "Please specify the image/distro which container should be based on(By default the image/distro will be set to Arch Linux.\nAll the image names can be found at the containers distros section in https://distrobox.it/compatibility/#containers-distros): ";
        getline(cin, image);
        if (image.empty()) {
            image = "quay.io/toolbx/arch-toolbox:latest";
        }
    }

    string cmd = "distrobox create " + name + " --image " + image;
    system(cmd.c_str());
    cout << "Container " << name << " created successfully.\n";
}

void upgrade_container() {
    string cmd = "distrobox upgrade " + name;
    system(cmd.c_str());
}

void view_container() {
    string cmd = "distrobox enter " + name;
    system(cmd.c_str());
}

void exec_command_container() {
    cout << command; 
    string cmd = "distrobox enter " + name + " -- " + command;
    system(cmd.c_str());
}

bool check_container_command_exists(const string& command) {
    string cmd = "distrobox enter " + name + " -- " + command + " --version > /dev/null 2>&1";
    int result = system(cmd.c_str());
    return result == 0;
}
void find_container_package_manager() {
    for (const auto& package_manager : package_managers) {
        if (check_container_command_exists(package_manager.first)) {
            package_manager_name = package_manager.first;             
            cout << "Package manager found as " << package_manager_name << " for container " << name << "\n";
        }
    }
}

void exec_action_for_container() {
    if (action == "create") {
        create_container();
    } else if (action == "delete") {
        delete_container();
    } else if (action == "view") {
        view_container();
    } else if (action == "upgrade") {
        upgrade_container();
    } else if (action == "list") {
        cout << list_container();
    } else if (action == "stop") {
        stop_container();
    } else if (action == "exec") {
        exec_command_container();
    } else if (action == "package_download") {
        exec_package_manager_operations();
    } else if (action == "package_search") {
        exec_package_manager_operations();
    } else if (action == "package_remove") {
        exec_package_manager_operations();
}  
}



