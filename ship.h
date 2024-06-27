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
#include <unordered_map>
#include <map>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <random>

using namespace std;

struct PackageManagerCommands {
    string base_command;
    string search_command;
    string install_command;
    string remove_command;
};

extern unordered_map<string, PackageManagerCommands> package_managers;

extern string mode;
extern string action;
extern string command;
extern string name;
extern string package_manager_name;
extern string packages;
extern string source;
extern string source_local;
extern string memory_limit;
extern string cpu_limit;
extern string image;
extern string iso_path; 
extern string disk_image_path; 

void show_help();
string exec(const char* cmd);
void exec_package_manager_operations();
vector<int> extract_numbers_with_prefix(const string& result,const string& prefix);
int get_next_available_number_in_command_output(const string& command);
int get_next_available_vm_number();
int get_next_available_container_number();
vector<string> split_string_by_line(const string& str);
string get_vm_state(const string &vm_name);
void send_file();
void receive_file();
void delete_old_snapshots();
void start_vm();
string list_vm();
void delete_vm();
void create_vm();
void save_vm();
void generate_vm_name();
void start_vm_with_confirmation_prompt();
void get_iso_source();
void create_disk_image();
void set_memory_limit();
void set_cpu_limit();
string generate_vm_xml();
void define_vm(const string& xml_filename);
void view_vm_console();
void view_vm_gui();
void view_vm();
void pause_vm();
void resume_vm();
void shutdown_vm();
bool exec_command_vm();
bool check_vm_command_exists();
void find_vm_package_manager();
void start_container();
string list_container();
void delete_container();
void create_container();
void stop_container();
void upgrade_container();
void view_container();
void stop_container();
void exec_command_container();
bool check_container_command_exists(const string& command);
void find_container_package_manager();
void process_operands(int argc, char *argv[]);
void exec_action();
void exec_action_for_container();
void exec_action_for_vm();
string get_absolute_path(const string &relative_path);
string generate_mac_address();
#endif // SHIP_H

