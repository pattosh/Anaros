{
  description = "Anaros C++26 reflection dev shell (GCC 16, Bazel, CMake)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachSystem [ "x86_64-linux" ] (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.gcc16
            pkgs.bazelisk
            pkgs.jdk
            pkgs.cmake
            pkgs.ninja
            pkgs.yaml-cpp
            pkgs.gtest
            pkgs.clang-tools
            pkgs.git
            pkgs.coreutils
          ];

          shellHook = ''
            export CMAKE_PREFIX_PATH="${pkgs.yaml-cpp}:${pkgs.gtest.dev}''${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
          '';
        };
      });
}
