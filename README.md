# GEARS

GEant4 Example Application with Rich features yet Small footprint.

Single-file Geant4 application (~570 SLOC) for radiation transport simulation.
Configured entirely via macro commands — no recompilation for geometry, physics, or source changes.

## Quick start

Requires [Nix](https://nixos.org/download/):

```sh
git clone <repo-url> && cd rG4-gears
nix develop
cmake -B gears/build -S gears && cmake --build gears/build
gears gears/tutorials/output/radiate.mac
```

## Features

### Output modes

Two modes, switchable via macro:

```
/output/mode event   # 6 columns: n, m, pdg0, k0, etotal, et
/output/mode step    # 29 columns: full step-point data + event summary
```

New event-level fields: `pdg0` (source particle PDG), `k0` (source kinetic energy), `etotal` (total energy deposit).

Step limit is configurable (`/output/maxSteps N`); overflow tags the event without killing the track.

### Sensitive volumes

Mark volumes via macro (replaces the old `(S)` naming convention):

```
/sensitive/add HPGe
/sensitive/add chamber
```

### Geometry

- [Text geometry description](tutorials/detector) (`.tg`) — recommended
- [GDML](tutorials/detector/GDML) import/export — for CAD toolchains
- [Boolean operations](tutorials/detector/boolean), [shell scripts](tutorials/detector/scripts)
- [Syntax highlighting](tutorials/detector/syntax) for Emacs, Vim, Sublime Text

### Physics

Switch physics lists without recompilation via `PHYSLIST` environment variable or `/physics_lists/select`:

```sh
PHYSLIST=FTFP_BERT_EMZ gears macro.mac
```

Tutorials for [alpha](tutorials/physics/alpha), [beta](tutorials/physics/beta), [gamma](tutorials/physics/gamma), [muon](tutorials/physics/muon), [neutron](tutorials/physics/neutron), [X-ray](tutorials/physics/X-ray).

### Sources

[GPS](tutorials/sources) (General Particle Source) — fully macro-driven:
energy spectra, angular distributions, spatial distributions, multi-source stacking.

### Scoring

Geant4 built-in mesh scoring (`/score/` commands) for energy deposition, flux, dose in 3D grids:

```
/score/create/boxMesh mesh
/score/mesh/boxSize 100 100 140 mm
/score/mesh/nBin 10 10 14
/score/quantity/energyDeposit eDep
/score/close
/run/beamOn 10000
/score/dumpQuantityToFile mesh eDep output.csv
```

### Analysis

Output is [ROOT](https://root.cern.ch) TTree format (no ROOT installation required).
Analyze with [uproot](https://pypi.org/project/uproot/) (Python) or ROOT:

```python
import uproot
t = uproot.open("gears.root")["t"]
print(t.keys())  # branch names
```

## Tutorials

See [tutorials/](tutorials) for comprehensive demos:

```sh
gears tutorials/output/demo_event.mac   # event mode
gears tutorials/output/demo_step.mac    # step mode
gears tutorials/output/demo_score.mac   # mesh scoring
gears tutorials/output/radiate.mac      # classic demo
```

## Environment

Provided by `flake.nix`:

| Component | Version | Purpose |
|-----------|---------|---------|
| Geant4 | 11.4.2 | Transport engine + all 15 data datasets |
| ROOT | 6.40.00 | PyROOT for analysis |
| Python | 3.14.7 | PyROOT + uproot/matplotlib (via uv) |
| cmake | — | Build system |
| GDML | enabled | Geometry import/export (xercesc) |
| ScoringManager | enabled | `/score/` mesh commands |

Python dependencies managed by uv (`pyproject.toml`): uproot, matplotlib, numpy.

## License

See [LICENSE](LICENSE).
