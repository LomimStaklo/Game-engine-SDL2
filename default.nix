{ pkgs ? import <nixpkgs> {} }:

let
  inherit (pkgs) lib;
in
pkgs.stdenv.mkDerivation {
  name = "SKF";
  src = lib.cleanSource ./.;
  buildInputs = with pkgs ; [ SDL2 sdl2-compat SDL2_image SDL2_mixer libwebp libtiff ];
  buildPhase = ''
    cc -o nob nob.c
    ./nob
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp ./SKF $out/bin
  '';
}
