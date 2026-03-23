{
  description = "4jcraft-nix package and dev shell";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

    shiggy = {
      url   = "github:4jcraft/shiggy/main";
      flake = false;
    };
    miniaudio = {
      url   = "https://github.com/mackron/miniaudio/archive/refs/tags/0.11.22.tar.gz";
      flake = false;
    };
    miniaudio-patch = {
      url   = "https://wrapdb.mesonbuild.com/v2/miniaudio_0.11.22-2/get_patch";
      flake = false;
    };
    "4jlibs" = {
      url   = "github:4jcraft/4jlibs/main";
      flake = false;
    };
    stb = {
      url   = "github:nothings/stb/master";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, ... } @ inputs:
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
    packages = forAllSystems ({ pkgs }: {
      default = pkgs.clangStdenv.mkDerivation {
        pname   = "4jcraft";
        version = "0.1.0";
        src     = ./.;

        dontUseCmakeConfigure = true;

        preConfigure = ''
          cp -r ${inputs.shiggy}    subprojects/shiggy
          cp -r ${inputs.miniaudio} subprojects/miniaudio
          chmod -R u+w subprojects/miniaudio
          unzip ${inputs."miniaudio-patch"} -d /tmp/miniaudio-patch/
          cp -r /tmp/miniaudio-patch/*/. subprojects/miniaudio/
          cat > subprojects/miniaudio.wrap <<'EOF'
          [wrap-file]
          directory = miniaudio

          [provide]
          dependency_names = miniaudio
          EOF
          cp -r ${inputs."4jlibs"}  subprojects/4jlibs
          cp -r ${inputs.stb}       subprojects/stb
          chmod -R u+w subprojects/
          cp subprojects/packagefiles/stb/meson.build subprojects/stb/meson.build
        '';

        buildInputs = with pkgs; [
          openssl.dev
          libGL
          libGLU
          SDL2
          zlib
        ];

        nativeBuildInputs = with pkgs; [
          cmake
          lld
          meson
          ninja
          pkg-config
          python3
          makeWrapper
          unzip
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
    });
  };
}
