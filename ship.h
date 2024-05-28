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

extern string mode;
extern string action;
extern string name;
extern string source;
extern string source_local;
extern string memory_limit;
extern string cpu_limit;
extern string image; 

void show_help();
string exec(const char* cmd);
vector<int> extract_numbers_with_prefix(const string& result,const string& prefix);
int get_next_available_number_in_command_output(const string& command);
int get_next_available_vm_number();
int get_next_available_container_number();
string get_vm_state(const string &vm_name);
void start_vm();
string list_vm();
void delete_vm();
void create_vm();
void view_vm();
void pause_vm();
void resume_vm();
void shutdown_vm();
void start_container();
string list_container();
void delete_container();
void create_container();
void stop_container();
void upgrade_container();
void view_container();
void stop_container();
void process_operands(int argc, char *argv[]);
void exec_action();
void exec_action_for_container();
void exec_action_for_vm();
string get_absolute_path(const string &relative_path);

#endif // SHIP_H

