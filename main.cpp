#include "ship.h"

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
