#include "ship.h"

string action;
string name;
string source;
string source_local;
string memory_limit;
string cpu_limit;

bool creating_vm = false;
bool starting_vm = false;
bool deleting_vm = false;
bool viewing_vm = false;
bool listing_vm  = false;

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

