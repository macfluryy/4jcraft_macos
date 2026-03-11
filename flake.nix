{
  description = "4jcraft-nix dev shell";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { nixpkgs, ... }:
  let
    allSystems = [
      "x86_64-linux"
      "aarch64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ];

    forAllSystems = f: nixpkgs.lib.genAttrs allSystems (system: f {
      pkgs = import nixpkgs { inherit system; };
    });
  in
  {
    devShells = forAllSystems ({ pkgs }:
      let
        libs = with pkgs; [
        openssl.dev
        libGL
        libGLU
        sdl2
        zlib
      ];
      packages = with pkgs; [
        clang
        lld
        cmake
        gnumake
        meson
        ninja
        pkg-config
      ];
      in
      {
        default = pkgs.mkShell {
          LD_LIBRARY_PATH = "/run/opengl-driver/lib";
          buildInputs = libs;
          nativeBuildInputs = packages;
        };
      }
    );
  };
}
