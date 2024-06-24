# Ship
- Ship if a powerful tool made for VoyageLinux designed to enhance the user's capability of using various environments for various needs mainly package management and installing software tailored to specific platforms. <br>
- It supports the use of virtual machines(virtual machine commands have been done using virsh)for providing isolated environments for installing package and doing whatever the user wants.It also supports the use of containers(container commands have been done with the help of distrobox),the containers do not provide isolated environments and should be used for installing packages from various distros.

## Setup and Installation 
### Arch based distros
```
git clone https://github.com/VoyageLinux/ship
sudo pacman -S libvirt qemu-base distrobox virsh
```

### Debian based distros
```
git clone https://github.com/VoyageLinux/ship
sudo apt-get install libvirt-daemon-system qemu-system distrobox
```
### Compiling
Pre compiled executables files are provided in the repo, but if you still wish to compile the program yourself, run the following commands.
```
cd Ship
make
```

## Running the program

Usage, Run:
```
./ship [OPTIONS] COMMAND [ARGS...]

Options:
  --help              Show this help message and exit
  -v, --verbose       Enable verbose output

Commands:
    create              Create a new virtual machine
    --name NAME           Set the name of the VM
    --source URL          Set the source URL of the VM
    --source-local PATH   Set the local source path of the VM
    --cpus NUMBER         Set the CPU limit of the VM
    --memory SIZE         Set the memory limit of the VM
    start NAME          Start the specified virtual machine
    delete NAME         Delete the specified virtual machine
    list                List all virtual machines
```
