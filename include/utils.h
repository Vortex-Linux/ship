#ifndef UTILS_H
#define UTILS_H

#include "ship_env.h" 

void show_help();
std::string trim_trailing_whitespaces(const std::string& str);
void system_exec(const std::string& cmd);
std::string exec(const std::string& cmd);
std::string list_vm();
std::string list_container();
std::string get_vm_state(const std::string &vm_name);
std::vector<int> extract_numbers_with_prefix(const std::string& result,const std::string& prefix);
int get_next_available_number_in_command_output(const std::string& command,const std::string& prefix);
int get_next_available_vm_number();
int get_next_available_container_number(); 
std::string get_absolute_path(const std::string &relative_path);
void exec_package_manager_operations();
std::string generate_random_number(int num_digits);
std::string generate_mac_address();
std::vector<std::string> split_string_by_line(const std::string& str);
std::string find_settings_file();
bool is_user_in_group(const std::string& group);
void add_user_to_group(const std::string& group);
bool file_exists(const std::string& file_path);
bool wait_for_file(const std::string& file_path, int timeout_seconds);
bool is_file_non_empty(const std::string& file_path);
bool wait_for_file_to_fill(const std::string& file_path, int timeout_seconds);
void move_file(const std::string& source, const std::string& destination);
std::string decompress_xz_file(const std::string& file_path);
std::string decompress_gzip_file(const std::string& file_path);
std::string decompress_bzip2_file(const std::string& file_path);
std::string decompress_lz4_file(const std::string& file_path);
std::string decompress_lzo_file(const std::string& file_path);
std::string decompress_lzma_file(const std::string& file_path);
std::string decompress_lzip_file(const std::string& file_path);
bool is_html_content(const std::string& url);
std::vector<std::string> get_links_from_page(const std::string& url);
void clear_split_files(const std::string& path);
void combine_split_files(const std::string& path, const std::string& combined_name);

#endif 

