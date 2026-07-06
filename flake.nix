{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-26.05";
  inputs.flake-parts.url = "github:hercules-ci/flake-parts";
  inputs.systems.url = "github:nix-systems/default";

  outputs = { self, flake-parts, systems, ... }@inputs: flake-parts.lib.mkFlake { inherit inputs; } {
    systems = import systems;
    perSystem = { lib, pkgs, ... }: {
      packages.default = pkgs.stdenv.mkDerivation {
        name = "SKF";
        src = lib.cleanSource ./.;
        buildInputs = with pkgs ; [ SDL2 sdl2-compat SDL2_image SDL2_mixer libwebp libtiff ];
        buildPhase = ''
          cc -o nob nob.c
          ./nob -assets-baked
        '';
        installPhase = ''
          mkdir -p $out/bin
          cp ./SKF $out/bin
        '';
      };
    };
  };
}
