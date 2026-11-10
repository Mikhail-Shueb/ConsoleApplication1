{
  description = "CGJ 2026 sample project - OpenGL C++ development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {inherit system;};

      # Windowing-system and GL runtime libraries. The projects now vendor and
      # build GLFW/GLEW from source (see labs/*/libraries), so we no longer ship
      # prebuilt glfw/glew; instead we provide what they need to build against
      # and dlopen at run time: the GL loader plus the Wayland and X11 backends.
      # Reused for both buildInputs and the runtime loader path.
      windowingDeps = with pkgs; [
        libGL
        wayland
        libxkbcommon
        libx11
        libxrandr
        libxinerama
        libxcursor
        libxi
      ];
    in {
      devShells.default = pkgs.mkShell {
        # Build tools that run on the host (compiler, generator, pkg-config).
        nativeBuildInputs = with pkgs; [
          gcc
          cmake
          gnumake
          pkg-config
          # Command runner for the projects' Justfile (just build / just run).
          just
          # clangd ships with clang-tools; pinning it here keeps the editor's
          # language server in sync with the gcc above and the project's C++
          # standard, so it resolves the standard library headers correctly.
          clang-tools
          # Needed to build GLFW's Wayland backend from source: wayland-scanner
          # generates C sources from the protocol XML shipped in wayland-protocols.
          wayland-scanner
          wayland-protocols
        ];

        # Headers/libraries the vendored dependencies compile and link against.
        buildInputs = windowingDeps;

        # GLFW is linked statically into the binary but dlopens the Wayland/X11
        # and GL runtimes lazily, so these must be on the loader path when a
        # freshly built binary is launched directly from the shell.
        LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath windowingDeps;

        shellHook = ''
          echo "CGJ dev shell: g++ $(g++ -dumpversion), cmake $(cmake --version | head -n1 | cut -d' ' -f3)"
          echo "Build:  cmake -S . -B build && cmake --build build"
        '';
      };
    });
}
