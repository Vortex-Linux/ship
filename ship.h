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

enum class ShipMode {
    UNKNOWN,
    VM,
    CONTAINER
};

// Allows for using the << operator with ShipMode
std::ostream& operator<<(std::ostream& os, const ShipMode& mode); 

enum class ShipAction {
    UNKNOWN,
    CREATE,
    DELETE,
    SAVE,
    VIEW,
    UPGRADE,
    LIST,
    START,
    PAUSE,
    STOP,
    RESUME,
    EXEC,
    PACKAGE_DOWNLOAD,
    PACKAGE_SEARCH,
    PACKAGE_REMOVE,
    RECEIVE,
    SEND,
    SHUTDOWN,
};

// allows for using the << operator with shipaction
std::ostream& operator<<(std::ostream& os, const ShipAction& action);

enum class TestedVM {
    UNKNOWN,
    tails,
    whonix,
    debian,
    ubuntu,
    arch,
    gentoo,
    fedora,
    centos,
    nix,
    alpine,
    Void,
    freebsd,
    openbsd,
    netbsd,
    dragonflybsd,
    windows,
    osx,
};

// allows for using the << operator with testedvm
std::ostream& operator<<(std::ostream& os, const TestedVM& vm);

struct ShipEnviornment {
    ShipMode mode;
    ShipAction action;
    string command;
    string name;
    string package_manager_name;
    string packages;
    TestedVM os;
    string source;
    string source_local;
    string memory_limit;
    string cpu_limit;
    string image;
    string iso_path;
    string disk_image_path;
};

extern ShipEnviornment ship_env;

void show_help();
string trim_trailing_whitespaces(const std::string& str);
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
void get_tested_vm();
void create_disk_image();
void set_memory_limit();
void set_cpu_limit();
string generate_vm_xml();
void define_vm(const string& xml_filename);
void configure_vm();
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

