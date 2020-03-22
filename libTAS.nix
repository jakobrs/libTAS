{ lib, mkDerivation,
  autoreconfHook, pkgconfig,
  file,
  SDL2,
  glibc, gcc-unwrapped, qtbase,
  libX11, libxcb, xcbutilkeysyms, xcbutilcursor,
  ffmpeg, alsaLib,
  fontconfig, freetype }:

let
  dualArch = false;

in mkDerivation rec {
  name = "libTAS-${version}";
  version = "0.0.0"; # This is debatable

  src = ./.;

  nativeBuildInputs = [ autoreconfHook pkgconfig ];
  buildInputs = [ xcbutilcursor SDL2 alsaLib ffmpeg ];
  #buildInputs = [ qtbase xlibs.xcbutilcursor.dev SDL2.dev alsaLib.dev ffmpeg.dev ];

  configureFlags = if dualArch then [ "--with-i386" ] else [];

  dontPatchELF = true;

  preFixup = ''
    ls -a $out/bin
    for file in $out/bin/{libTAS,libtas.so}; do
      echo "Setting rpath of $file"
      echo "Previous rpath: $(patchelf --print-rpath $file)"
      patchelf \
        --set-rpath ${lib.makeLibraryPath [
          glibc gcc-unwrapped.lib qtbase
          libX11 libxcb xcbutilkeysyms xcbutilcursor
          ffmpeg alsaLib
          fontconfig.lib freetype
        ]} \
        $file
    done
  '';

  postFixup = ''
    ls -a $out/bin
    wrapProgram $out/bin/libTAS --suffix PATH : ${lib.makeBinPath [ file ]}
  '';
}
