{
  description = "A devshell with Bazel";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = [
            pkgs.gcc16
            pkgs.bazelisk # Recommended over pkgs.bazel to manage versions via .bazelversion
            pkgs.jdk      # Often required for Bazel
            pkgs.gtest
          ];
        };
      });
}
