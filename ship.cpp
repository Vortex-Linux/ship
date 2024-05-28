#include "ship.h"

string mode;
string action;
string name;
string source;
string source_local;
string memory_limit;
string cpu_limit;
string image;

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

