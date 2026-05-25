{ pkgs }:
pkgs.buildEnv {
  name = "anaros-cc-toolchain";
  paths = [
    pkgs.gcc16
    pkgs.binutils
    pkgs.coreutils
  ];
}
