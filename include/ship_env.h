#ifndef SHIP_ENV_H
#define SHIP_ENV_H

#include "headers.h"
#include "utils.h"
#include "main.h"
#include "ship.h"
#include "container_operations.h"
#include "vm_operations.h"

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
    OPTIMIZE, 
    COMPRESS, 
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

#endif

