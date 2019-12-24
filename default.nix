{ pkgs ? import <nixpkgs> {} }:

pkgs.libsForQt5.callPackage ./libTAS.nix {}
