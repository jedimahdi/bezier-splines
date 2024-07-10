{ pkgs ? import <nixpkgs> { } }:
pkgs.mkShell {
  buildInputs = with pkgs; [
    glfw
    raylib
    vulkan-headers
    vulkan-loader
    vulkan-tools
    xorg.libXcursor
    xorg.libXrandr
    xorg.libXinerama
  ];
  LD_LIBRARY_PATH = "$LD_LIBRARY_PATH:${
    with pkgs;
    lib.makeLibraryPath [
      libGL
      xorg.libX11
      xorg.libXi
      vulkan-loader
      glfw
      raylib
    ]
  }";
}
