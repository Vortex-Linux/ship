#ifndef SHIP_H
#define SHIP_H

#include <iostream>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <string>
#include <climits>
#include <fstream>
#include <regex>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <array>
#include <unordered_map>
#include <map>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <random>
#include <sys/stat.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>

extern boost::property_tree::ptree pt;

struct PackageManagerCommands {
    std::string base_command;
    std::string search_command;
    std::string install_command;
    std::string remove_command;
};

extern std::unordered_map<std::string, PackageManagerCommands> package_managers;

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
    RESTART,
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
    alpine,
    freebsd,
    openbsd,
    netbsd,
    dragonflybsd,
    windows,
};

// allows for using the << operator with testedvm
std::ostream& operator<<(std::ostream& os, const TestedVM& vm);

struct ShipEnviornment {
    ShipMode mode;
    ShipAction action;
    std::string command;
    std::string name;
    std::string package_manager_name;
    std::string packages;
    TestedVM os;
    std::string source;
    std::string source_local;
    std::string memory_limit;
    std::string cpu_limit;
    std::string image;
    std::string iso_path;
    std::string disk_image_path;
};

extern std::string ship_lib_path; 

extern ShipEnviornment ship_env;

void show_help();
std::string find_settings_file();
std::string trim_trailing_whitespaces(const std::string& str);
void system_exec(const std::string& cmd);
std::string exec(const std::string& cmd);
void exec_package_manager_operations();
std::vector<int> extract_numbers_with_prefix(const std::string& result,const std::string& prefix);
int get_next_available_number_in_command_output(const std::string& command);
int get_next_available_vm_number();
int get_next_available_container_number();
std::vector<std::string> split_string_by_line(const std::string& str);
std::string get_vm_state(const std::string &vm_name);
void send_container_file();
void receive_container_file();
void send_vm_file();
void receive_vm_file();
void delete_old_snapshots();
void pass_password_to_tmux();
void run_startup_commands();
void wait_for_vm_ready();
std::string find_network_address_vm();
void attach_xpra();
void start_vm();
void restart_vm();
std::string list_vm();
std::string get_vm_image_paths();
void clean_vm_resources();
bool vm_exists(const std::string& vm_name);
void delete_vm();
void create_vm();
void save_vm();
void generate_vm_name();
void start_vm_with_confirmation_prompt();
void get_iso_source();
void print_available_tested_vms();
std::string get_tested_vm_link(const std::string &vm_name);
void tested_vm_information();
void set_tested_vm(const std::string &vm_name);
void get_tested_vm();
void create_disk_image();
void set_memory_limit();
void set_cpu_limit();
std::string generate_vm_xml();
void define_vm(const std::string& xml_filename);
void configure_vm();
void view_vm_console();
void view_vm_gui();
void view_vm();
void pause_vm();
void resume_vm();
void shutdown_vm();
void system_command_vm(const std::string& command);
bool exec_command_vm(const std::string& command);
bool check_vm_command_exists();
void find_vm_package_manager();
void start_container();
std::string list_container();
void delete_container();
void create_container();
void get_tested_container();
void stop_container();
void upgrade_container();
void view_container();
void stop_container();
void exec_command_container();
bool check_container_command_exists(const std::string& command);
void find_container_package_manager();
void process_operands(int argc, char *argv[]);
void exec_action();
void exec_action_for_container();
void exec_action_for_vm();
bool is_user_in_group(const std::string& group);
void add_user_to_group(const std::string& group);
void restart_systemctl_service(const std::string& service_name);
std::string get_absolute_path(const std::string &relative_path);
std::string generate_mac_address();
bool file_exists(const std::string& file_path);
bool wait_for_file(const std::string& file_path, int timeout_seconds);
bool wait_for_file_to_fill(const std::string& file_path, int timeout_seconds);
bool is_file_non_empty(const std::string& file_path);
#endif // SHIP_H

