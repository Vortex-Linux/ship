#include "ship.h"

void stop_container() {
    string cmd = "distrobox stop " + ship_env.name;
    string result = exec(cmd.c_str());
    system(cmd.c_str());

}

void delete_container() {
    string cmd = "distrobox rm " + ship_env.name;
    system(cmd.c_str());
}

void create_container() {
    if (ship_env.name.empty()) {
        int next_container_number = get_next_available_container_number();
        ship_env.name = "container_" + to_string(next_container_number);
        cout << "Please specify the name of this container(leaving this blank will set the name to " << ship_env.name << ")";
        string name_given;
        getline(cin, name_given);
        if(!name_given.empty()) {
            ship_env.name = name_given; 
        }
    }

    if (ship_env.image.empty()) {
        cout << "Please specify the image/distro which container should be based on(By default the image/distro will be set to Arch Linux.\nAll the image names can be found at the containers distros section in https://distrobox.it/compatibility/#containers-distros): ";
        getline(cin, ship_env.image);
        if (ship_env.image.empty()) {
            ship_env.image = "quay.io/toolbx/arch-toolbox:latest";
        }
    }

    string cmd = "distrobox create " + ship_env.name + " --image " + ship_env.image;
    system(cmd.c_str());
    cout << "Container " << ship_env.name << " created successfully.\n";
}

void upgrade_container() {
    string cmd = "distrobox upgrade " + ship_env.name;
    system(cmd.c_str());
}

void view_container() {
    string cmd = "distrobox enter " + ship_env.name;
    system(cmd.c_str());
}

void exec_command_container() {
    cout << ship_env.command; 
    string cmd = "distrobox enter " + ship_env.name + " -- " + ship_env.command;
    system(cmd.c_str());
}

bool check_container_command_exists(const string& command) {
    string cmd = "distrobox enter " + ship_env.name + " -- " + command + " --version > /dev/null 2>&1";
    int result = system(cmd.c_str());
    return result == 0;
}

void find_container_package_manager() {
    for (const auto& package_manager : package_managers) {
        if (check_container_command_exists(package_manager.first)) {
            ship_env.package_manager_name = package_manager.first;             
            cout << "Package manager found as " << ship_env.package_manager_name << " for container " << ship_env.name << "\n";
        }
    }
}

void exec_action_for_container() {
    if (!ship_env.package_manager_name.empty()) {
        
    }
    switch (ship_env.action) {
        case ShipAction::CREATE:
            create_container();
            break;
        case ShipAction::DELETE:
            delete_container();
            break; 
        case ShipAction::VIEW:
            view_container();
            break;
        case ShipAction::UPGRADE:
            upgrade_container();
            break;
        case ShipAction::LIST:
            cout << list_container();
            break;
        case ShipAction::STOP:
            stop_container();
            break;
        case ShipAction::EXEC:
            exec_command_container();
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
            cout << "Invalid action for container\n";
            break;
    }
}



