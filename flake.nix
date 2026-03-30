{
  description = "4jcraft nix-package and dev-shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }@inputs:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        packages.default = pkgs.clangStdenv.mkDerivation {
          pname = "4jcraft";
          version = "0.1.0";
          src = ./.;

          dontFixup = true;
          dontUseCmakeConfigure = true;

          nativeBuildInputs = with pkgs; [
            lld
            makeWrapper
            meson
            ninja
            pkg-config
            python3
            unzip
          ];

          buildInputs = with pkgs; [
            openssl.dev
            libGL
            libGLU
            glm
            SDL2
            zlib
          ];

          installPhase = ''
            mkdir -p $out/share/4jcraft
            cp -r Minecraft.Client/. $out/share/4jcraft/

            mkdir -p $out/bin
            makeWrapper $out/share/4jcraft/Minecraft.Client $out/bin/4jcraft \
              --run "cd $out/share/4jcraft"
          '';

          meta = {
            description = "4JCraft";
            platforms = pkgs.lib.platforms.unix;
          };
        };

        devShells.default =
          pkgs.mkShell.override
            {
              stdenv = pkgs.clangStdenv;
            }
            {
              inputsFrom = [ self.packages.${system}.default ];

              packages = with pkgs; [
                clang-tools
                lldb
                valgrind
                include-what-you-use
                ccache
              ];

              shellHook = ''
                export CC="ccache clang"
                export CXX="ccache clang++"
              '';
            };
      }
    );
}
