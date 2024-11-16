#ifndef VM_OPERATIONS_H
#define VM_OPERATIONS_H

void pass_password_to_tmux();
void wait_for_vm_ready();
void run_startup_commands();
std::string find_network_address_vm();
void attach_xpra(const std::string &username,const std::string &password);
void start_vm();
void restart_vm();
void view_vm_console();
void view_vm_gui();
void view_vm();
void pause_vm();
void resume_vm();
void delete_old_snapshots();
void save_vm();
void shutdown_vm();
std::string get_vm_image_paths();
bool vm_exists(const std::string& vm_name);
void delete_vm();
void process_source_file();
void create_vm();
std::string generate_vm_xml();
void define_vm(const std::string& xml_filename);
void start_vm_with_confirmation_prompt();
void download_iso();
void get_iso_source();
void print_available_tested_vms();
std::string get_tested_vm_link(const std::string &vm_name);
void tested_vm_information();
void set_tested_vm(const std::string &vm_name);
void get_tested_vm();
void create_disk_image();
std::string get_disk_image_path();
void convert_disk_image(const std::string &source_image, const std::string &dest_image, const std::string &options);
void convert_to_compact_image(const std::string &original_image_path, const std::string &compact_image_path);
void convert_to_compressed_image(const std::string &original_image_path, const std::string &compact_image_path);
void delete_disk_image(const std::string &image_path);
void detach_disk(const std::string &vm_name, const std::string &disk_name);
void attach_disk(const std::string &vm_name, const std::string &disk_path, const std::string &disk_name);
std::string generate_unique_image_path();
void replace_vm_disk(const std::string &vm_name, const std::string &new_disk_path, const std::string &disk_target);
void create_optimized_disk_image();
void generate_vm_name();
void set_memory_limit();
void set_cpu_limit();
void configure_vm();
void system_command_vm(const std::string& command);
bool exec_command_vm(const std::string& command);
bool check_vm_command_exists(const std::string& command);
void find_vm_package_manager();
void send_vm_file();
void receive_vm_file();
void exec_action_for_vm();

#endif 

