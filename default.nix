{ pkgs ? import <nixpkgs> {}, multiArch ? true }:

pkgs.libsForQt5.callPackage ./libTAS.nix { inherit multiArch; }
