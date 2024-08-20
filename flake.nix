{
  description = "Development environment for Ship";

  # 1) You need the `kvm` kernel module for virtualization to work.
  #    Choose either `kvm-intel` or `kvm-amd` depending on your CPU.
  #    Make sure you have virtualization enabled in your BIOS.
  #    ```
  #    boot.kernelModules = [ ... "kvm-intel" ... ];
  #    ```
  # 
  # 2) Enable `libvirtd`
  #    ```
  #    virtualisation.libvirtd.enable = true;
  #    ```
  # 
  # 3) Add the user to the `docker` and `libvirtd` groups:
  #    ```nix
  #    users.users.my_username = {
  #      ...
  #      extraGroups = [ ... "docker" "libvirtd" ... ];
  #      ...
  #    };
  #    ```
  # 
  # 4) After rebooting your machine, run:
  #    ```
  #    nix develop
  #    ```

  inputs = {
    nixpkgs = { url = "github:nixos/nixpkgs/nixos-unstable"; };
  };

  outputs = { self, nixpkgs }: 
  let 
    supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
    
    # Helper function to generate an attrset '{ x86_64-linux = f "x86_64-linux"; ... }'.
    forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

    # Nixpkgs instantiated for supported system types.
    pkgsFor = forAllSystems (system: import nixpkgs { inherit system; });
  in
  {
    devShells = forAllSystems (system: {
      default =
        pkgsFor.${system}.mkShell.override {
          stdenv = pkgsFor.${system}.gcc14Stdenv;
        } {

          name = "ship-dev-shell";
          hardeningDisable = ["all"];
          buildInputs = with pkgsFor.${system}; [
            libgcc
            boost
          ];
          packages = with pkgsFor.${system}; [
            tmux
            aria2
            docker
            croc
            distrobox
            cloud-init
            qemu_kvm
            lynx
          ];
          shellHook = ''
              echo "Enabling default network for the vm"
              sudo virsh net-start default 2>/dev/null || true
          '';
        };
    });
  };
}
