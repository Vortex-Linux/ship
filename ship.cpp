#include "ship.h"

unordered_map<string, PackageManagerCommands> package_managers = {
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

string mode;
string action;
string command;
string name;
string package_manager_name;
string packages;
string source;
string source_local;
string memory_limit;
string cpu_limit;
string image;
string iso_path;
string disk_image_path;

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
                command = command +  " " + argv[j];
            }
            break;
        }

        if (fetching_name) {
            name = argv[i];
            fetching_name = false;
            
            if (action == "exec") {
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
                packages += argv[j];
            }
            if (strcmp(argv[i], "--parameters") != 0 || strcmp(argv[i], "-p") != 0) {
                break;
            }
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

        if (strcmp(argv[i], "--container") == 0 || strcmp(argv[i], "-ctr") == 0) {
            mode = "container";
            continue;
        }

        if (strcmp(argv[i], "--virtual-machine") == 0 || strcmp(argv[i], "-vm") == 0 ) {
            mode = "vm";
            continue;
        }

        if (strcmp(argv[i], "create") == 0 && action == "") {
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
            action = "start";
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "delete") == 0 && action == "") {
            action = "delete";
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "list") == 0 && action == "") {
            action = "list";
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "view") == 0 || strcmp(argv[i], "enter") == 0)&& action == "") {
            action = "view";
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "view") == 0 && action == "") {
            action = "view";
            action_index = i;
            continue;
        }
        if (strcmp(argv[i], "shutdown") == 0 && action == "" && mode=="vm") {
            action = "shutdown";
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "stop") == 0 && action == "") {
            action = "stop";
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "pause") == 0 || strcmp(argv[i], "suspend") == 0) && action == "") {
            action = "pause";
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "resume") == 0 && action == "") {
            action = "resume";
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "upgrade") == 0 && action == "") {
            action = "upgrade";
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "exec") == 0 || strcmp(argv[i], "run") == 0) && action == "") {
            action = "exec";
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "package_download") == 0 || strcmp(argv[i], "download_packages") == 0 ) && action == "") {
            action = "package_download";
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "package_remove") == 0 || strcmp(argv[i], "remove_packages") == 0 )&& action == "") {
            action = "package_remove";
            action_index = i;
            continue;
        }

        if ((strcmp(argv[i], "package_search") == 0 || strcmp(argv[i], "search_packages") == 0 )&& action == "") {
            action = "package_search";
            action_index = i;
            continue;
        }

        if (strcmp(argv[i], "--command") == 0 && action == "exec") {
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

        if (strcmp(argv[i], "--aur") == 0 && strcmp(argv[i], "-aur") == 0) {
            package_manager_name = "pacman";
            continue;
        }

        if (strcmp(argv[i], "--dnf") == 0 && strcmp(argv[i], "-dnf") == 0) {
            package_manager_name = "dnf";
            continue;
        }

        if (strcmp(argv[i], "--apt") == 0 && strcmp(argv[i], "-apt") == 0) {
            package_manager_name = "apt";
            continue;
        }

        if (strcmp(argv[i], "--nix") == 0 && strcmp(argv[i], "-nix") == 0) {
            package_manager_name = "nix";
            continue;
        }

        if (i - 1 == action_index) {
            name = argv[i];
            continue;
        }

        if (action == "") {
            cout << "Ship found unknown operand " << argv[i] << " for entity " << mode << "\n";
            cout << "For more information try ship --help\n";
        } else {
            cout << "Ship found unknown operand " << argv[i] << " for action " << action << "\n";
        }

        exit(1);
    }
}

void exec_action() {
    if (mode == "vm") {
        exec_action_for_vm();
    } else if (mode == "container") {
        exec_action_for_container();
    } else {
        cout << "Are you using this action for vms " << name << "? (y/n,Note:The default behaviour is that the action is assumed to be for containers): ";
        string confirm;
        getline(cin, confirm);

        if (confirm != "y" && confirm != "Y") {
            exec_action_for_container();
        } else {
            exec_action_for_vm();
        }

    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "ship: missing operand\n";
        cout << "For more information try ship --help\n";
        return 1;
    }
    process_operands(argc, argv);
    exec_action();
    return 0;
}

