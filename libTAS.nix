{ stdenv, multiStdenv, lib,

  # nativeBuildInputs
  wrapQtAppsHook, # Because multiarch + qt5 is hard
  autoreconfHook, pkgconfig,
  git, # Used in configure to generate a version string or something like that

  # buildInputs
  xcbutilcursor, SDL2, alsaLib, ffmpeg,

  # Even more dependencies
  file, # Used to get information about the architecture of a file
  glibc, gcc-unwrapped, qtbase,
  libX11, libxcb, xcbutilkeysyms,
  fontconfig, freetype,
  libGL,

  # Multiarch is enabled whenever possible
  multiArch ? stdenv.hostPlatform.isx86_64, pkgsi686Linux
}:

let
  relevantStdenv = if multiArch then multiStdenv else stdenv;

in relevantStdenv.mkDerivation rec {
  pname = "libTAS";
  version = "0.0.0"; # This is debatable

  src = lib.cleanSource ./.;

  nativeBuildInputs = [ autoreconfHook pkgconfig wrapQtAppsHook git ];
  buildInputs = [
    xcbutilcursor SDL2 alsaLib ffmpeg
  ] ++ lib.optionals multiArch [
    pkgsi686Linux.xorg.xcbutilcursor
    pkgsi686Linux.SDL2
    pkgsi686Linux.alsaLib
    pkgsi686Linux.ffmpeg
    # Why are these required here but not above?
    pkgsi686Linux.fontconfig
    pkgsi686Linux.freetype
  ];

  dontStrip = true; # Segfaults, bug in patchelf
  dontPatchELF = true; # We'll do this ourselves

  patches = [
    ./libtaspath.patch
  ];

  # Note that this builds an extra .so file in the same derivation
  # Ideally the library and executable might be split into separate derivations,
  # but this is easier for now
  configureFlags = lib.optional multiArch "--with-i386";

  postPatch = ''
    substituteInPlace src/program/main.cpp \
      --subst-var out
  '';

  postInstall = ''
    mkdir -p $out/lib
    mv $out/bin/libtas*.so $out/lib/
  '';

  /*
  preFixup = ''
    for file in $out/{bin/libTAS,lib/libtas.so}; do
      patchelf \
        --set-rpath ${lib.makeLibraryPath [
          glibc gcc-unwrapped.lib qtbase
          libX11 libxcb xcbutilkeysyms xcbutilcursor
          ffmpeg alsaLib
          fontconfig.lib freetype
          libGL
        ]} \
        $file
    done
  '';
  */

  postFixup = ''
    wrapProgram $out/bin/libTAS --suffix PATH : ${lib.makeBinPath [ file ]}
  '';

  meta = {
    platforms = [ "x86_64-linux" "i686-linux" ];
    description = "GNU/Linux software to (hopefully) give TAS tools to native games";
    license = lib.licenses.gpl3Only;
  };
}
