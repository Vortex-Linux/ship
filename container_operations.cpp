#include "ship.h"

void stop_container() {
    std::string cmd = "distrobox stop " + ship_env.name;
    std::string result = exec(cmd);
    system_exec(cmd);

}

void delete_container() {
    std::string cmd = "distrobox rm " + ship_env.name;
    system_exec(cmd);
}

void create_container() {
    if (ship_env.name.empty()) {
        int next_container_number = get_next_available_container_number();
        ship_env.name = "container_" + std::to_string(next_container_number);
        std::cout << "Please specify the name of this container(leaving this blank will set the name to " << ship_env.name << ")";
        std::string name_given;
        getline(std::cin, name_given);
        if(!name_given.empty()) {
            ship_env.name = name_given; 
        }
    }

    if (ship_env.image.empty()) {
        get_tested_container();
    }

    std::string cmd = "distrobox create " + ship_env.name + " --image " + ship_env.image;
    system_exec(cmd);
    std::cout << "Container " << ship_env.name << " created successfully.\n";
}

void get_tested_container() {
    std::map<std::string, std::map<std::string, std::string>> container_data = {
        {"AlmaLinux (Toolbox)", {
            {"8", "quay.io/toolbx-images/almalinux-toolbox:8"},
            {"9", "quay.io/toolbx-images/almalinux-toolbox:9"},
            {"latest", "quay.io/toolbx-images/almalinux-toolbox:latest"}
        }},
        {"Alpine (Toolbox)", {
            {"3.16", "quay.io/toolbx-images/alpine-toolbox:3.16"},
            {"3.17", "quay.io/toolbx-images/alpine-toolbox:3.17"},
            {"3.18", "quay.io/toolbx-images/alpine-toolbox:3.18"},
            {"3.19", "quay.io/toolbx-images/alpine-toolbox:3.19"},
            {"edge", "quay.io/toolbx-images/alpine-toolbox:edge"},
            {"latest", "quay.io/toolbx-images/alpine-toolbox:latest"}
        }}, 
        {"AmazonLinux (Toolbox)", {
            {"2", "quay.io/toolbx-images/amazonlinux-toolbox:2"},
            {"2023", "quay.io/toolbx-images/amazonlinux-toolbox:2023"},
            {"latest", "quay.io/toolbx-images/amazonlinux-toolbox:latest"}
        }},
        {"Archlinux (Toolbox)", {
            {"latest", "quay.io/toolbx/arch-toolbox:latest"}
        }},
        {"Bazzite Arch", {
            {"latest", "ghcr.io/ublue-os/bazzite-arch:latest"},
            {"gnome", "ghcr.io/ublue-os/bazzite-arch-gnome:latest"}
        }},
        {"CentOS (Toolbox)", {
            {"stream8", "quay.io/toolbx-images/centos-toolbox:stream8"},
            {"stream9", "quay.io/toolbx-images/centos-toolbox:stream9"},
            {"latest", "quay.io/toolbx-images/centos-toolbox:latest"}
        }},
        {"Debian (Toolbox)", {
            {"10", "quay.io/toolbx-images/debian-toolbox:10"},
            {"11", "quay.io/toolbx-images/debian-toolbox:11"},
            {"12", "quay.io/toolbx-images/debian-toolbox:12"},
            {"testing", "quay.io/toolbx-images/debian-toolbox:testing"},
            {"unstable", "quay.io/toolbx-images/debian-toolbox:unstable"},
            {"latest", "quay.io/toolbx-images/debian-toolbox:latest"}
        }},
        {"Fedora (Toolbox)", {
            {"37", "registry.fedoraproject.org/fedora-toolbox:37"},
            {"38", "registry.fedoraproject.org/fedora-toolbox:38"},
            {"39", "registry.fedoraproject.org/fedora-toolbox:39"},
            {"40", "registry.fedoraproject.org/fedora-toolbox:40"},
            {"rawhide", "registry.fedoraproject.org/fedora-toolbox:rawhide"}
        }},
        {"openSUSE (Toolbox)", {
            {"latest", "registry.opensuse.org/opensuse/distrobox:latest"}
        }},
        {"RedHat (Toolbox)", {
            {"8", "registry.access.redhat.com/ubi8/toolbox"},
            {"9", "registry.access.redhat.com/ubi9/toolbox"},
            {"latest", "quay.io/toolbx-images/rhel-toolbox:latest"}
        }},
        {"Rocky Linux (Toolbox)", {
            {"8", "quay.io/toolbx-images/rockylinux-toolbox:8"},
            {"9", "quay.io/toolbx-images/rockylinux-toolbox:9"},
            {"latest", "quay.io/toolbx-images/rockylinux-toolbox:latest"}
        }},
        {"Ubuntu (Toolbox)", {
            {"16.04", "quay.io/toolbx/ubuntu-toolbox:16.04"},
            {"18.04", "quay.io/toolbx/ubuntu-toolbox:18.04"},
            {"20.04", "quay.io/toolbx/ubuntu-toolbox:20.04"},
            {"22.04", "quay.io/toolbx/ubuntu-toolbox:22.04"},
            {"latest", "quay.io/toolbx/ubuntu-toolbox:latest"}
        }},
        {"AlmaLinux", {
            {"8", "docker.io/library/almalinux:8"},
            {"8-minimal", "docker.io/library/almalinux:8-minimal"},
            {"9", "docker.io/library/almalinux:9"},
            {"9-minimal", "docker.io/library/almalinux:9-minimal"}
        }},
        {"Alpine Linux", {
            {"3.15", "docker.io/library/alpine:3.15"},
            {"3.16", "docker.io/library/alpine:3.16"},
            {"3.17", "docker.io/library/alpine:3.17"},
            {"3.18", "docker.io/library/alpine:3.18"},
            {"3.19", "docker.io/library/alpine:3.19"},
            {"edge", "docker.io/library/alpine:edge"},
            {"latest", "docker.io/library/alpine:latest"}
        }},
        {"AmazonLinux", {
            {"1", "public.ecr.aws/amazonlinux/amazonlinux:1"},
            {"2", "public.ecr.aws/amazonlinux/amazonlinux:2"},
            {"2023", "public.ecr.aws/amazonlinux/amazonlinux:2023"}
        }},
        {"Archlinux", {
            {"latest", "docker.io/library/archlinux:latest"}
        }},
        {"CentOS Stream", {
            {"8", "quay.io/centos/centos:stream8"},
            {"9", "quay.io/centos/centos:stream9"}
        }},
        {"CentOS", {
            {"7", "quay.io/centos/centos:7"}
        }},
        {"Chainguard Wolfi", {
            {"latest", "cgr.dev/chainguard/wolfi-base:latest"}
        }},
        {"ClearLinux", {
            {"latest", "docker.io/library/clearlinux:latest"},
            {"base", "docker.io/library/clearlinux:base"}
        }},
        {"Crystal Linux", {
            {"latest", "registry.getcryst.al/crystal/misc/docker:latest"}
        }},
        {"Debian", {
            {"7", "docker.io/debian/eol:wheezy"},
            {"8", "docker.io/library/debian:buster"},
            {"9", "docker.io/library/debian:bullseye-backports"},
            {"10", "docker.io/library/debian:bookworm-backports"},
            {"11", "docker.io/library/debian:stable-backports"},
            {"12", "docker.io/library/debian:stable-backports"}
        }},
        {"Debian Testing", {
            {"latest", "docker.io/library/debian:testing"},
            {"backports", "docker.io/library/debian:testing-backports"}
        }},
        {"Debian Unstable", {
            {"latest", "docker.io/library/debian:unstable"}
        }},
        {"deepin", {
            {"20", "docker.io/linuxdeepin/apricot"},
            {"23", "docker.io/linuxdeepin/beige"}
        }},
        {"Fedora", {
            {"36", "quay.io/fedora/fedora:36"},
            {"37", "quay.io/fedora/fedora:37"},
            {"38", "quay.io/fedora/fedora:38"},
            {"39", "quay.io/fedora/fedora:39"},
            {"rawhide", "quay.io/fedora/fedora:rawhide"}
        }},
        {"Gentoo Linux", {
            {"rolling", "docker.io/gentoo/stage3:latest"}
        }},
        {"KDE neon", {
            {"latest", "invent-registry.kde.org/neon/docker-images/plasma:latest"}
        }},
        {"Kali Linux", {
            {"rolling", "docker.io/kalilinux/kali-rolling:latest"}
        }},
        {"Mint", {
            {"21.1", "docker.io/linuxmintd/mint21.1-amd64"}
        }},
        {"Neurodebian", {
            {"nd100", "docker.io/library/neurodebian:nd100"}
        }},
        {"openSUSE", {
            {"Leap", "registry.opensuse.org/opensuse/leap:latest"},
            {"Tumbleweed", "registry.opensuse.org/opensuse/tumbleweed:latest"}
        }},
        {"Oracle Linux", {
            {"7", "container-registry.oracle.com/os/oraclelinux:7"},
            {"7-slim", "container-registry.oracle.com/os/oraclelinux:7-slim"},
            {"8", "container-registry.oracle.com/os/oraclelinux:8"},
            {"8-slim", "container-registry.oracle.com/os/oraclelinux:8-slim"},
            {"9", "container-registry.oracle.com/os/oraclelinux:9"},
            {"9-slim", "container-registry.oracle.com/os/oraclelinux:9-slim"}
        }},
        {"RedHat (UBI)", {
            {"7", "registry.access.redhat.com/ubi7/ubi"},
            {"8", "registry.access.redhat.com/ubi8/ubi"},
            {"9", "registry.access.redhat.com/ubi9/ubi"}
        }},
        {"Rocky Linux", {
            {"8", "rockylinux/rockylinux:8"},
            {"9", "rockylinux/rockylinux:9"},
            {"latest", "rockylinux/rockylinux:latest"}
        }},
        {"Ubuntu", {
            {"16.04", "ubuntu:16.04"},
            {"18.04", "ubuntu:18.04"},
            {"20.04", "ubuntu:20.04"},
            {"22.04", "ubuntu:22.04"},
            {"latest", "ubuntu:latest"}
        }}
};

    while (true) {
        std::cout << "Please specify a container from our tested and configured containers (use 'help' to list available containers): ";
        std::string distro;
        std::getline(std::cin, distro);

        if (distro == "help") {
            std::cout << "The available tested and configured containers are: " << std::endl;
            for (const auto& entry : container_data) {
                std::cout << entry.first << std::endl;
            }
            continue;
        } 

        auto distro_it = container_data.find(distro);
        if (distro_it == container_data.end()) {
            std::cout << "The specified container is not available as a tested and configured container." << std::endl;
            continue;
        }

        while (true) {
            std::cout << "Please specify a version for the distro (use 'help' to list available versions): ";
            std::string version;
            std::getline(std::cin, version);

            if (version == "help") {
                std::cout << "The available versions for " << distro << " are: " << std::endl;
                for (const auto& version_entry : distro_it->second) {
                    std::cout << version_entry.first << std::endl;
                }
                continue;
            }

            auto version_it = distro_it->second.find(version);
            if (version_it == distro_it->second.end()) {
                std::cout << "The specified version is not available as a tested and configured container." << std::endl;
            } else {
                ship_env.image = version_it->second; 
                std::cout << "Selected container image: " << ship_env.image << std::endl;
                return; 
            }
        }
    }
}


void upgrade_container() {
    std::string cmd = "distrobox upgrade " + ship_env.name;
    system_exec(cmd);
}

void view_container() {
    std::string cmd = "distrobox enter " + ship_env.name;
    system_exec(cmd);
}

void exec_command_container() {
    std::cout << ship_env.command; 
    std::string cmd = "distrobox enter " + ship_env.name + " -- " + ship_env.command;
    system_exec(cmd);
}

bool check_container_command_exists(const std::string& command) {
    std::string cmd = "distrobox enter " + ship_env.name + " -- " + command + " --version > /dev/null 2>&1";
    int result = system(cmd.c_str());
    return result == 0;
}

void find_container_package_manager() {
    for (const auto& package_manager : package_managers) {
        if (check_container_command_exists(package_manager.first)) {
            ship_env.package_manager_name = package_manager.first;             
            std::cout << "Package manager found as " << ship_env.package_manager_name << " for container " << ship_env.name << "\n";
        }
    }
}

void send_container_file() {
    std::string image = "/tmp/" + ship_env.name;

    std::string make_container_image_cmd = "docker commit " + ship_env.name + " " + ship_env.name;
    std::cout << exec(make_container_image_cmd) << std::endl;

    std::string make_container_tar_file_cmd = "docker save -o " + image + " " + ship_env.name;
    std::cout << exec(make_container_tar_file_cmd) << std::endl;

    std::string send_container_image_cmd = "croc send " + image;
    std::cout << exec(send_container_image_cmd) << std::endl;

    std::string image_cleanup_cmd = "docker rmi " + ship_env.name;
    std::cout << exec(image_cleanup_cmd) << std::endl;
}

void receive_container_file() {
    std::string code;
    std::cout << "Please type the secret code: ";
    std::getline(std::cin, code);

    std::string set_croc_secret_cmd = "export CROC_SECRET=" + code;
    std::cout << exec(set_croc_secret_cmd) << std::endl;

    std::string receive_container_cmd = "croc recv";
    std::cout << exec(receive_container_cmd) << std::endl;

    std::string find_container_image_cmd = "ls -t | head -1";
    std::string image = exec(find_container_image_cmd);

    std::string load_container_image_cmd = "docker load -i " + image;
    std::cout << exec(load_container_image_cmd) << std::endl;

    std::string create_container_cmd = "distrobox create --name " + image + " --image " + image;
    std::cout << exec(create_container_cmd) << std::endl;
}

void exec_action_for_container() {
    if (!ship_env.package_manager_name.empty()) {
        
    }
    switch (ship_env.action) {
        case ShipAction::CREATE:
            create_container();
            break;
        case ShipAction::DELETE:
            delete_container();
            break; 
        case ShipAction::VIEW:
            view_container();
            break;
        case ShipAction::UPGRADE:
            upgrade_container();
            break;
        case ShipAction::LIST:
            std::cout << list_container();
            break;
        case ShipAction::STOP:
            stop_container();
            break;
        case ShipAction::EXEC:
            exec_command_container();
            break;
        case ShipAction::PACKAGE_DOWNLOAD:
        case ShipAction::PACKAGE_SEARCH:
        case ShipAction::PACKAGE_REMOVE:
            exec_package_manager_operations();
            break;
        case ShipAction::RECEIVE:
            receive_container_file();
            break;
        case ShipAction::SEND:
            send_container_file();
            break;
        default:
            std::cout << "Invalid action for container\n";
            break;
    }
}



