# Ship
- Ship if a powerful tool made for Vortex Linux designed to enhance the user's capability of using various environments for various needs mainly package management and installing software tailored to specific platforms. <br>
- It supports the use of virtual machines(virtual machines are managed using libvirt,virsh,qemu and other utilities for them)for providing isolated/unisolated environments for installing package and doing whatever the user wants.It also supports the use of containers(Containers are managed using distrobox and docker),the containers do not provide isolated environments and should be used for installing packages from various distros.

## Setup and Installation 

### Note:(Ship is the superutility of Vortex Linux but can still be used with other distros. If you are using Vortex Linux, Ship is available by default, and you don't have to download it.)

### Important Prerequisites

- Ensure that virtualization is enabled in the BIOS.
- The kernel must have the "kvm" module enabled for virtualization to function properly.

### Debian based distros
```
git clone https://github.com/Vortex-Linux/ship.git
sudo apt install qemu-kvm libvirt-daemon-system virt-viewer libboost-all-dev lynx aria2 tmux
curl https://raw.githubusercontent.com/89luca89/distrobox/main/install | sudo sh
wget https://github.com/schollz/croc/releases/download/v9.4.2/croc_9.4.2_Linux-64bit.deb
sudo dpkg -i croc-*.deb
sudo usermod -a -G libvirt $user
sudo systemctl enable --now cloud-init-local.service
sudo systemctl enable --now cloud-init.service
sudo systemctl enable --now cloud-config.service
sudo systemctl enable --now cloud-final.service
```
### Fedora based distros
```
git clone https://github.com/Vortex-Linux/ship.git
curl https://raw.githubusercontent.com/89luca89/distrobox/main/install | sudo sh
sudo dnf install qemu croc @virtualization boost-devel lynx aria2 tmux
sudo usermod -a -G libvirt $user
sudo systemctl enable --now cloud-init-local.service
sudo systemctl enable --now cloud-init.service
sudo systemctl enable --now cloud-config.service
sudo systemctl enable --now cloud-final.service
```

### Arch based distros
```
git clone https://github.com/Vortex-Linux/ship.git
sudo pacman -S libvirt qemu-base distrobox docker croc virt-viewer boost lynx aria2 tmux
sudo usermod -a -G libvirt $user
sudo systemctl enable --now cloud-init-local.service
sudo systemctl enable --now cloud-init.service
sudo systemctl enable --now cloud-config.service
sudo systemctl enable --now cloud-final.service
```

### NixOS

You need the `kvm` kernel module for virtualization to work.
Choose either `kvm-intel` or `kvm-amd` depending on your CPU.
```
boot.kernelModules = [ ... "kvm-intel" ... ];
```

Enable `libvirtd`
```
virtualisation.libvirtd.enable = true;
```

Add the user to the `docker` and `libvirtd` groups:
```nix
users.users.my_username = {
  ...
  extraGroups = [ ... "docker" "libvirtd" ... ];
  ...
};
```

After rebooting your machine, run:
```
nix develop
```

### Compiling
Pre compiled executables files are provided in the repo, but in the case that its not compatible or you are want to contribute and want to try your changes you can compile the binaries yourself,run these commands:
```
cd ship
make clean
make
```

## Running the program

Usage, Run:
```
./ship [OPTIONS] COMMAND [ARGS...]

Options:
  --help                          Show this help message and exit
  --virtual-machine or --vm        Specify action is for VM
  --container or --ctr             Specify action is for container
  --name NAME                     Specifies the name of the container or VM an action should be executed on

Commands:
  vm
    create
      --source URL                Set the source URL of the VM
      --source-local PATH         Set the local source path of the VM
      --cpus NUMBER               Set the CPU limit of the VM
      --memory or -mem SIZE       Set the memory limit of the VM
    start NAME                    Start the specified virtual machine
    delete NAME                   Delete the specified virtual machine
    list                          List all virtual machines
    view or enter NAME            Shows a console interface or a full GUI of the virtual machine
    pause NAME                    Pause the specified virtual machine
    resume NAME                   Resume the specified virtual machine if it's paused
    save NAME                     Take a snapshot of the specified virtual machine
    shutdown NAME                 Shut down the specified virtual machine
    exec NAME                     Execute the given command in the console of the specified virtual machine
      --command COMMAND           Set the command to be executed
    package_download or download_packages NAME
                                  Download the specified package using the package manager of the specified virtual machine
      --package PACKAGE           Set the package to be downloaded
    package_remove or remove_packages NAME
                                  Remove the specified package using the package manager of the specified virtual machine
      --package PACKAGE           Set the package to be removed
    package_search or search_packages NAME
                                  Search for the specified package using the package manager of the specified virtual machine
      --package PACKAGE           Set the package to be searched
    send NAME                     Share the specified virtual machine to an end user (protected by a secret code)
    receive                       Receive the virtual machine shared by an end user (protected by a secret code)

  container
    create NAME                   Create a new container with the specified name
    delete NAME                   Delete the specified container
    view or enter NAME            Show a console interface
    upgrade NAME                  Upgrade the specified container
    list                          List all containers
    stop NAME                     Stop the specified container
    exec NAME                     Execute the given command in the console of the specified container
      --command COMMAND           Set the command to be executed
    package_download or download_packages NAME
                                  Download the specified package using the package manager of the specified container
      --package PACKAGE           Set the package to be downloaded
    package_remove or remove_packages NAME
                                  Remove the specified package using the package manager of the specified container
      --package PACKAGE           Set the package to be removed
    package_search or search_packages NAME
                                  Search for the specified package using the package manager of the specified container
      --package PACKAGE           Set the package to be searched
    send NAME                     Share the specified container to an end user (protected by a secret code)
    receive                       Receive the container shared by an end user (protected by a secret code)
```

