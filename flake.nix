{
  description= "Wizard Duel";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      # Systems we want to support
      supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      
      # Helper to create an environment for each system
      forAllSystems = f: nixpkgs.lib.genAttrs supportedSystems (system: f (import nixpkgs { inherit system; }));
    in
    {
      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          name = "wizard-duel-env";

          # Build tools
          nativeBuildInputs = with pkgs; [
            gnumake
            gcc
            pkg-config
          ];

          # Libraries required for the game
          buildInputs = with pkgs; [
            raylib
            enet
            openssl
            mesa
            libGL
            xorg.libX11
            xorg.libXcursor
            xorg.libXinerama
            xorg.libXi
            xorg.libXrandr
            xorg.libXft
          ] ++ (if pkgs.stdenv.isDarwin then [
            pkgs.darwin.apple_sdk.frameworks.Cocoa
            pkgs.darwin.apple_sdk.frameworks.IOKit
            pkgs.darwin.apple_sdk.frameworks.CoreVideo
          ] else []);

          shellHook = ''
            export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath (with pkgs; [ raylib enet openssl libGL xorg.libX11 ])}:$LD_LIBRARY_PATH"
            echo "Wizard Duel Environment Loaded!"
            echo "Libraries: raylib, enet, openssl, GL"
            echo "Run 'make' to build or 'make run' to play."
          '';
        };
      });
    };
}
