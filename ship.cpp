#include "ship.h"

boost::property_tree::ptree pt;

std::ostream& operator<<(std::ostream& os, const ShipMode& mode) {
    switch (mode) {
        case ShipMode::VM:
            return os << "VM";
        case ShipMode::CONTAINER:
            return os << "Container";
        default:
            return os << "Unknown";
    }
}

std::ostream& operator<<(std::ostream& os, const ShipAction& action) {
   switch (action) {
        case ShipAction::CREATE:
            return os << "Create";
        case ShipAction::DELETE:
            return os << "Delete";
        case ShipAction::SAVE:
            return os << "Save";
        case ShipAction::VIEW:
            return os << "View";
        case ShipAction::UPGRADE:
            return os << "Upgrade";
        case ShipAction::LIST:
            return os << "List";
        case ShipAction::START:
            return os << "Start";
        case ShipAction::PAUSE:
            return os << "Pause";
        case ShipAction::STOP:
            return os << "Stop";
        case ShipAction::RESUME:
            return os << "Resume";
        case ShipAction::EXEC:
            return os << "Exec";
        case ShipAction::PACKAGE_DOWNLOAD:
            return os << "Package Download";
        case ShipAction::PACKAGE_SEARCH:
            return os << "Package Search";
        case ShipAction::PACKAGE_REMOVE:
            return os << "Package Remove";
        case ShipAction::RECEIVE:
            return os << "Receive";
        case ShipAction::SEND:
            return os << "Send";
        case ShipAction::SHUTDOWN:
            return os << "Shutdown";
        default:
            return os << "Unknown";
   }
}

std::unordered_map<std::string, PackageManagerCommands> package_managers = {
    {"ship", {"sudo ship", "sudo ship search", "sudo ship install", "sudo ship remove"}},
    {"apk", {"sudo apk", "sudo apk search", "sudo apk add", "sudo apk del"}},
    {"apt-get", {"sudo apt-get", "sudo apt-get search", "sudo apt-get install", "sudo apt-get remove"}},
    {"dnf", {"sudo dnf", "sudo dnf search", "sudo dnf install", "sudo dnf remove"}},
    {"zypper", {"sudo zypper", "sudo zypper search", "sudo zypper install", "sudo zypper remove"}},
    {"emerge", {"sudo emerge", "sudo emerge --search", "sudo emerge", "sudo emerge --unmerge"}},
    {"pacman", {"sudo pacman", "sudo pacman -Ss", "sudo pacman -S", "sudo pacman -R"}},
    {"yum", {"sudo yum", "sudo yum search", "sudo yum install", "sudo yum remove"}},
    {"apx", {"sudo apx", "sudo apx search", "sudo apx install", "sudo apx remove"}},
    {"shards", {"sudo shards", "sudo shards search", "sudo shards install", "sudo shards remove"}},
    {"pkgtool", {"sudo pkgtool", "sudo pkgtool search", "sudo pkgtool install", "sudo pkgtool remove"}},
    {"xbps", {"sudo xbps", "sudo xbps-query -Rs", "sudo xbps-install", "sudo xbps-remove"}},
    {"paludis", {"sudo paludis", "sudo cave search", "sudo cave resolve -x", "sudo cave uninstall"}},
    {"urpmi", {"sudo urpmi", "sudo urpmq", "sudo urpmi", "sudo urpme"}},
    {"tce-load", {"sudo tce-load", "sudo tce-ab search", "sudo tce-load -wi", "sudo tce-remove"}},
    {"equo", {"sudo equo", "sudo equo search", "sudo equo install", "sudo equo remove"}},
    {"tazpkg", {"sudo tazpkg", "sudo tazpkg search", "sudo tazpkg install", "sudo tazpkg remove"}}
};

std::ostream& operator<<(std::ostream& os, const TestedVM& vm) {
    switch (vm) {
        case TestedVM::tails:
            return os << "tails";
        case TestedVM::whonix:
            return os << "whonix";
        case TestedVM::debian:
            return os << "debian";
        case TestedVM::ubuntu:
            return os << "ubuntu";
        case TestedVM::arch:
            return os << "arch";
        case TestedVM::gentoo:
            return os << "gentoo";
        case TestedVM::fedora:
            return os << "fedora";
        case TestedVM::nix:
            return os << "nix";
        case TestedVM::alpine:
            return os << "alpine";
        case TestedVM::centos:
            return os << "centos";
        case TestedVM::Void:
            return os << "void";
        case TestedVM::freebsd:
            return os << "freebsd";
        case TestedVM::openbsd:
            return os << "openbsd";
        case TestedVM::netbsd:
            return os << "netbsd";
        case TestedVM::dragonflybsd:
            return os << "dragonflybsd";
        case TestedVM::windows:
            return os << "windows";
        case TestedVM::osx:
            return os << "osx";
        default:
            return os << "";
    }
}

// Initializing the struct with empty strings
ShipEnviornment ship_env = {
    ShipMode::UNKNOWN,  // mode
    ShipAction::UNKNOWN,  // action
    "",  // command
    "",  // name
    "",  // package_manager_name
    "",  // packages
    TestedVM::UNKNOWN, // os 
    "",  // source
    "",  // source_local
    "",  // memory_limit
    "",  // cpu_limit
    "",  // image
    "",  // iso_path
    "",  // disk_image_path
};

void process_operands(int argc, char *argv[]) {
    bool fetching_command = false;
    bool fetching_name = false;
    bool fetching_packages = false;
    bool fetching_source = false;
    bool fetching_local_source = false;
    bool fetching_cpu_limit = false;
    bool fetching_memory_limit = false;
    int action_index = INT_MAX;


    for (int i = 1; i < argc; ++i) {
        if (fetching_command) {
            for (int j=i+1;j<argc;j++) {
                ship_env.command = ship_env.command +  " " + argv[j];
            }
            break;
        }

        if (fetching_name) {
            ship_env.name = argv[i];
            fetching_name = false;
            
            if (ship_env.action == ShipAction::EXEC) {
                fetching_command = true;
            }
            continue;
        }

        if (fetching_packages) {
            for (int j=i;j<argc;j++) {
                if (strcmp(argv[j], "--parameters") == 0 || strcmp(argv[j], "-p") == 0) {
                    i = j;
                    break;
                }
                ship_env.packages += argv[j];
            }
            if (strcmp(argv[i], "--parameters") != 0 || strcmp(argv[i], "-p") != 0) {
                break;
            }
        }

        if (fetching_source) {
            ship_env.source = argv[i];
            ship_env.source_local = "";
            fetching_source = false;
            continue;
        }

        if (fetching_local_source) {
            ship_env.source_local = argv[i];
            ship_env.source = "";
            fetching_local_source = false;
            continue;
        }

        if (fetching_cpu_limit) {
            ship_env.cpu_limit = argv[i];
            fetching_cpu_limit = false;
            continue;
        }

        if (fetching_memory_limit) {
            ship_env.memory_limit = argv[i];
            fetching_memory_limit = false;
            continue;
        }

        if (strcmp(argv[i], "--help") == 0) {
            show_help();
            exit(0);
        }

        if (strcmp(argv[i], "--container") == 0 || strcmp(argv[i], "-ctr") == 0) {
            ship_env.mode = ShipMode::CONTAINER;
            continue;
        }

        if (strcmp(argv[i], "--virtual-machine") == 0 || strcmp(argv[i], "-vm") == 0 ) {
            ship_env.mode = ShipMode::VM;
            continue;
        }

        if (strcmp(argv[i], "create") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::CREATE;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "--name") == 0 && action_index!=INT_MAX) {
            fetching_name = true;
            continue;
        }

        if (strcmp(argv[i], "--source") == 0 && ship_env.action== ShipAction::CREATE) {
            fetching_source = true;
            continue;
        }

        if (strcmp(argv[i], "--source-local") == 0 && ship_env.action==ShipAction::CREATE) {
            fetching_local_source = true;
            continue;
        }

        if (strcmp(argv[i], "--cpus") == 0 && ship_env.action==ShipAction::CREATE) {
            fetching_cpu_limit = true;
            continue;
        }

        if ((strcmp(argv[i], "--memory") == 0 || strcmp(argv[i], "--mem") == 0) && ship_env.action==ShipAction::CREATE) {
            fetching_memory_limit = true;
            continue;
        }

        if (strcmp(argv[i], "start") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::START;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "delete") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::DELETE;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "list") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::LIST;
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "view") == 0 || strcmp(argv[i], "enter") == 0)&& ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::VIEW;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "view") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::VIEW;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "save") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::SAVE;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "shutdown") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::SHUTDOWN;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "stop") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::STOP;
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "pause") == 0 || strcmp(argv[i], "suspend") == 0) && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::PAUSE;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "resume") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::RESUME;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "upgrade") == 0 && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::UPGRADE;
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "exec") == 0 || strcmp(argv[i], "run") == 0) && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::EXEC;
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "package_download") == 0 || strcmp(argv[i], "download_packages") == 0 ) && ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::PACKAGE_DOWNLOAD;
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "package_remove") == 0 || strcmp(argv[i], "remove_packages") == 0 )&& ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::PACKAGE_REMOVE;
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "package_search") == 0 || strcmp(argv[i], "search_packages") == 0 )&& ship_env.action == ShipAction::UNKNOWN) {
            ship_env.action = ShipAction::PACKAGE_SEARCH;
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "receive") == 0 && ship_env.action == ShipAction::UNKNOWN)) {
            ship_env.action = ShipAction::RECEIVE;
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "send") == 0 && ship_env.action == ShipAction::UNKNOWN)) {
            ship_env.action = ShipAction::SEND;
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "--command") == 0 && ship_env.action == ShipAction::EXEC) {
            fetching_command = true;
            continue;
        }

        if ((strcmp(argv[i], "--parameters") == 0 || strcmp(argv[i], "-p") == 0 )) {
            fetching_command = true;
            continue;
        }

        if (strcmp(argv[i], "--packages") == 0) {
            fetching_packages = true;
            continue;
        }

        if (strcmp(argv[i], "--aur") == 1 && strcmp(argv[i], "-aur") == 0) {
            ship_env.package_manager_name = "pacman";
            continue;
        }

        if (strcmp(argv[i], "--apk") == 1 && strcmp(argv[i], "-aur") == 0) {
            ship_env.package_manager_name = "pacman";
            continue;
        }

        if (strcmp(argv[i], "--dnf") == 0 && strcmp(argv[i], "-dnf") == 0) {
            ship_env.package_manager_name = "dnf";
            continue;
        }

        if (strcmp(argv[i], "--apt") == 0 && strcmp(argv[i], "-apt") == 0) {
            ship_env.package_manager_name = "apt";
            continue;
        }

        if (strcmp(argv[i], "--nix") == 0 && strcmp(argv[i], "-nix") == 0) {
            ship_env.package_manager_name = "nix";
            continue;
        }

        if (i - 1 == action_index) {
            ship_env.name = argv[i];
            continue;
        }

        if (ship_env.action == ShipAction::UNKNOWN) {
            std::cout << "Ship found unknown operand " << argv[i] << " for entity " << ship_env.mode << "\n";
            std::cout << "For more information try ship --help\n";
        } else {
            std::cout << "Ship found unknown operand " << argv[i] << " for action " << ship_env.action << "\n";
        }

        exit(1);
    }
}

void exec_action() {
    if (ship_env.mode == ShipMode::VM) {
        exec_action_for_vm();
    } else if (ship_env.mode == ShipMode::CONTAINER) {
        exec_action_for_container();
    } else {
        std::cout << "Are you using this action for vms " << ship_env.name << "? (y/n,Note:The default behaviour is that the action is assumed to be for containers): ";
        std::string confirm;
        getline(std::cin, confirm);

        if (confirm != "y" && confirm != "Y") {
            ship_env.mode = ShipMode::CONTAINER;
            exec_action_for_container();
        } else {
            ship_env.mode = ShipMode::VM;
            exec_action_for_vm();
        }

    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "ship: missing operand\n";
        std::cout << "For more information try ship --help\n";
        return 1;
    }
    process_operands(argc, argv);
    exec_action();
    return 0;
}

