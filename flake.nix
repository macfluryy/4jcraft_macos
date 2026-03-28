{
  description = "4jcraft nix-package and dev-shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    shiggy = {
      url = "github:4jcraft/shiggy/main";
      flake = false;
    };

    "4jlibs" = {
      url = "github:4jcraft/4jlibs/main";
      flake = false;
    };

    miniaudio = {
      url = "https://github.com/mackron/miniaudio/archive/refs/tags/0.11.22.tar.gz";
      flake = false;
    };

    miniaudio-patch = {
      url = "https://wrapdb.mesonbuild.com/v2/miniaudio_0.11.22-2/get_patch";
      flake = false;
    };

    stb = {
      url = "github:nothings/stb/master";
      flake = false;
    };

    simdutf = {
      url = "github:simdutf/simdutf";
      flake = false;
    };
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

          dontUseCmakeConfigure = true;

          # 4jcraft - Meson expects this subprojects structure
          postUnpack = ''
            mkdir -p $sourceRoot/subprojects

            cp -r ${inputs.shiggy} $sourceRoot/subprojects/shiggy
            cp -r ${inputs."4jlibs"} $sourceRoot/subprojects/4jlibs
            cp -r ${inputs.stb} $sourceRoot/subprojects/stb
            cp -r ${inputs.simdutf} $sourceRoot/subprojects/simdutf
            cp -r ${inputs.miniaudio} $sourceRoot/subprojects/miniaudio

            chmod -R u+w $sourceRoot/subprojects
          '';

          # 4jcraft - `stb` and `simdutf` patches
          postPatch = ''
            cp subprojects/packagefiles/stb/meson.build subprojects/stb/meson.build
            cp subprojects/packagefiles/simdutf/meson.build subprojects/simdutf/meson.build
            cp subprojects/packagefiles/simdutf/meson.options subprojects/simdutf/meson.options

            unzip ${inputs.miniaudio-patch} -d miniaudio-patch-tmp
            cp -r miniaudio-patch-tmp/*/. subprojects/miniaudio/

            cat > subprojects/miniaudio.wrap <<EOF
            [wrap-file]
            directory = miniaudio
            [provide]
            dependency_names = miniaudio
            EOF
          '';

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
