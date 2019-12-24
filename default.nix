{ pkgs ? import <nixpkgs> {} }:

pkgs.callPackage ./libTAS.nix {}
