#ifndef SHIP_H
#define SHIP_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <fstream>
#include <regex>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <array>
#include <sstream>

using namespace std;

extern string action;
extern string name;
extern string source;
extern string source_local;
extern string memory_limit;
extern string cpu_limit;

extern bool creating_vm;
extern bool starting_vm;
extern bool deleting_vm;
extern bool viewing_vm;
extern bool listing_vm;

void show_help();
string exec(const char* cmd);
string list_vm();
int get_next_available_vm_number();
string get_vm_state(const string &vm_name);
void start_vm();
void delete_vm();
void create_vm();
void process_operands(int argc, char *argv[]);
void exec_action();
string get_absolute_path(const string &relative_path);

#endif // SHIP_H

