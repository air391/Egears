# GEARS

GEant4 Example Application with Rich features yet Small footprint.

Single-file Geant4 application (~570 SLOC) for radiation transport simulation.
Configured entirely via macro commands — no recompilation for geometry, physics, or source changes.

## Quick start

Requires [Nix](https://nixos.org/download/):

```sh
git clone git@github.com:air391/Egears.git && cd Egears
nix develop
./example/run.sh                           # default run
./example/run.sh --help                    # see all options
```

## Example

One example, multiple modes — see [example/](example):

```sh
./example/run.sh                           # .tg geometry, event mode, 50k events
./example/run.sh --mode step               # step mode (29 columns)
./example/run.sh --geometry gdml           # GDML geometry
./example/run.sh --vis                     # Qt visualization
./example/run.sh --parallel 8              # 8 parallel jobs
```

## Features

### Output modes

```
/output/mode event   # 6 columns: n, m, pdg0, k0, etotal, et
/output/mode step    # 29 columns: full step-point data + event summary
```

### Sensitive volumes

```
/sensitive/add crystal
```

### Geometry

- Text geometry (`.tg`) — recommended
- GDML import/export — for CAD toolchains

### Physics

Switch physics lists without recompilation:

```sh
PHYSLIST=FTFP_BERT_EMZ ./example/run.sh
```

### Sources

GPS (General Particle Source) — fully macro-driven:
energy spectra, angular distributions, spatial distributions.

### Scoring

Geant4 built-in mesh scoring (`/score/` commands).

### Visualization

Qt-based: `OGLSQt`, `OGLIQt`, `TSGQt`, `TSGQT_ZB`, `RayTracerQt`.
X11-based: `OGLIX`, `OGLSX`, `TSG_X11_GLES`, `TSG_X11_ZB`.

## Environment

Provided by `flake.nix`:

| Component | Version | Purpose |
|-----------|---------|---------|
| Geant4 | 11.4.2 | Transport engine + all 15 data datasets + Qt |
| ROOT | 6.40.00 | PyROOT for analysis |
| Python | 3.14.7 | PyROOT + uproot/matplotlib (via uv) |
| cmake | — | Build system |
| GDML | enabled | Geometry import/export (xercesc) |

Python dependencies managed by uv (`pyproject.toml`): uproot, matplotlib, numpy.

## License

See [LICENSE](LICENSE).
