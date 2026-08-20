{
  description = "GEARS (Geant4) development environment — same flake+uv pattern as rG4";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system);
    in
    {
      devShells = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          g4 = pkgs.geant4;
          # All Geant4 datasets so physics processes can locate data files.
          # Filter out the non-derivation attrs (override, overrideDerivation).
          g4data = builtins.filter pkgs.lib.isDerivation (builtins.attrValues g4.data);
          # Single Python interpreter (3.14). ROOT in nixpkgs is built
          # against python3.14, so PyROOT works out of the box; the same
          # interpreter is used by the uv-managed uproot workflow.
          py = pkgs.python314;
          # C++ runtime + zlib. The uv-managed venv installs numpy/matplotlib
          # as manylinux wheels, which expect libstdc++ and libz at runtime.
          runtimeLibs = [
            pkgs.stdenv.cc.cc.lib
            pkgs.zlib
          ];
        in
        {
          default = pkgs.mkShell {
            packages =
              [
                g4
                pkgs.root
                pkgs.cmake
                pkgs.gnumake
                py
              ]
              ++ runtimeLibs
              ++ g4data;

            # Expose the interpreter under an explicit name so scripts can
            # find it regardless of PATH ordering. Also expose the C++/z
            # runtimes via LD_LIBRARY_PATH so the uv-managed venv (manylinux
            # wheels like numpy) can locate them.
            shellHook = ''
              ln -sf ${py}/bin/python3.14 "$PWD/.python"
              export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath runtimeLibs}:''${LD_LIBRARY_PATH:-}"

              echo "Geant4  : ${g4.version}"
              echo "ROOT    : ${pkgs.root.version}"
              echo "Python  : ${py.version} (PyROOT + uv/uproot)"
              echo "App     : gears (single-file Geant4 application)"
              echo ""
              echo "Build & run:"
              echo "  cd gears && cmake -B build && cmake --build build"
              echo "  cd build && ./gears ../tutorials/output/radiate.mac   # from tutorials/output/"
              echo ""
              echo "Analyze output:"
              echo "  uv sync --python .python && .venv/bin/python <script>.py"
            '';
          };
        }
      );
    };
}
