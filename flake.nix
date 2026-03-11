{
  description = "4jcraft-nix package and dev shell";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs, ... }:
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
    packages = forAllSystems ({ pkgs }:
      {
        default = pkgs.clangStdenv.mkDerivation {
          pname   = "4jcraft";
          version = "0.1.0";

          src = ./.;

          buildInputs = with pkgs; [
            openssl.dev
            libGL
            libGLU
            SDL2
            zlib
          ];

          nativeBuildInputs = with pkgs; [
            lld
            meson
            ninja
            pkg-config
            python3
            makeWrapper
          ];

          installPhase = ''
            mkdir -p $out/share/4jcraft
            cp -r Minecraft.Client/. $out/share/4jcraft/

            mkdir -p $out/bin
            makeWrapper $out/share/4jcraft/Minecraft.Client \
              $out/bin/4jcraft \
              --run "cd $out/share/4jcraft"
          '';

          meta = {
            description = "4JCraft";
            platforms   = pkgs.lib.platforms.linux;
          };
        };
      }
    );
  };
}
