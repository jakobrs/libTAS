{ pkgs, mkDerivation, automake, autoconf, autoreconfHook, qtbase, pkgconfig, xlibs, SDL2, alsaLib, ffmpeg }:

let
  dualArch = false;

in mkDerivation rec {
  name = "libTAS-${version}";
  version = "0.0.0"; # This is debatable

  src = ./.;

  nativeBuildInputs = [ automake autoconf autoreconfHook pkgconfig ];
  buildInputs = [ qtbase xlibs.xcbutilcursor.dev SDL2.dev alsaLib.dev ffmpeg.dev ];

  configureFlags = if dualArch then [ "--with-i386" ] else [];
}
