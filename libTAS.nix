{ pkgs, stdenv, automake, autoconf, autoreconfHook, qt5, pkgconfig, xlibs, SDL2, alsaLib, ffmpeg }:

let
  dualArch = false;

in stdenv.mkDerivation {
  name = "libTAS";
  version = "0.0.0"; # This is debatable

  src = ./.;

  nativeBuildInputs = [ automake autoconf autoreconfHook pkgconfig ];
  buildInputs = [ automake autoconf qt5.qtbase xlibs.xcbutilcursor.dev SDL2.dev alsaLib.dev ffmpeg.dev ];

#  preConfigure = ''
#    autoconf
#    autoheader
#    automake --add-missing
#  '';

  configureFlags = if dualArch then [ "--with-i386" ] else [];
}
