#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace po = boost::program_options;

struct ShipEnvironment {
    enum class ShipMode { CONTAINER, VM };
    enum class ShipAction { 
        UNKNOWN, CREATE, START, DELETE, LIST, VIEW, SAVE, SHUTDOWN, 
        STOP, PAUSE, RESUME, UPGRADE, EXEC, PACKAGE_DOWNLOAD, 
        PACKAGE_REMOVE, PACKAGE_SEARCH, RECEIVE, SEND 
    };

    ShipMode mode = ShipMode::CONTAINER;
    ShipAction action = ShipAction::UNKNOWN;
    std::string name;
    std::string source;
    std::string source_local;
    std::string cpu_limit;
    std::string memory_limit;
    std::string command;
    std::vector<std::string> packages;
    std::string package_manager_name;
};

ShipEnvironment ship_env;

void process_operands(int argc, char* argv[]) {
    po::options_description desc;
    desc.add_options()
        ("container,ctr", po::bool_switch())
        ("virtual-machine,vm", po::bool_switch())
        ("action", po::value<std::string>())
        ("name", po::value<std::string>())
        ("source", po::value<std::string>())
        ("source-local", po::value<std::string>())
        ("cpus", po::value<std::string>())
        ("memory,mem", po::value<std::string>())
        ("command", po::value<std::vector<std::string>>()->multitoken())
        ("packages", po::value<std::vector<std::string>>()->multitoken())
        ("package-manager", po::value<std::string>())
    ;

    po::positional_options_description p;
    p.add("action", 1).add("name", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << "\n";
        exit(1);
    }

    if (vm.count("container")) {
        ship_env.mode = ShipEnvironment::ShipMode::CONTAINER;
    } else if (vm.count("virtual-machine")) {
        ship_env.mode = ShipEnvironment::ShipMode::VM;
    }

    if (vm.count("action")) {
        std::string action = vm["action"].as<std::string>();
        // Map string to ShipAction enum
        // You'll need to implement this mapping based on your requirements
    }

    if (vm.count("name")) {
        ship_env.name = vm["name"].as<std::string>();
    }

    if (vm.count("source")) {
        ship_env.source = vm["source"].as<std::string>();
    }

    if (vm.count("source-local")) {
        ship_env.source_local = vm["source-local"].as<std::string>();
    }

    if (vm.count("cpus")) {
        ship_env.cpu_limit = vm["cpus"].as<std::string>();
    }

    if (vm.count("memory")) {
        ship_env.memory_limit = vm["memory"].as<std::string>();
    }

    if (vm.count("command")) {
        auto cmd = vm["command"].as<std::vector<std::string>>();
        ship_env.command = "";
        for (const auto& c : cmd) {
            ship_env.command += c + " ";
        }
        ship_env.command = ship_env.command.substr(0, ship_env.command.length() - 1);
    }

    if (vm.count("packages")) {
        ship_env.packages = vm["packages"].as<std::vector<std::string>>();
    }

    if (vm.count("package-manager")) {
        ship_env.package_manager_name = vm["package-manager"].as<std::string>();
    }
}
