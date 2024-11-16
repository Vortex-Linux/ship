#ifndef CONTAINER_OPERATIONS_H
#define CONTAINER_OPERATIONS_H

#include "ship_env.h" 

void stop_container();
void delete_container();
void create_container();
void get_tested_container();
void upgrade_container();
void view_container();
void exec_command_container(const std::string& command);
bool check_container_command_exists(const std::string& command);
void find_container_package_manager();
void send_container_file();
void receive_container_file();
void exec_action_for_container();

#endif 

