# GEARS Example

One example, multiple modes. Everything controlled by `run.sh`.

## Quick start

```sh
nix develop
cd example
./run.sh                                 # default: .tg, event mode, 50k events
```

## Modes

### Geometry: .tg vs GDML

```sh
./run.sh                                 # text geometry (.tg)
./run.sh --geometry gdml                 # GDML (auto-generated from .tg on first run)
```

### Output: event vs step

```sh
./run.sh --mode event                    # 6 columns: n, m, pdg0, k0, etotal, et
./run.sh --mode step                     # 29 columns: full step-point data
```

### Visualization

```sh
./run.sh --vis                           # Qt OpenGL window (requires display)
./run.sh --vis --geometry gdml --mode step
```

### Parallel

```sh
./run.sh --parallel 8                    # 8 independent jobs, different seeds
./run.sh --parallel 4 --events 100000    # 4 jobs x 100k events each
# merge: hadd output.root output_*.root
```

### Physics list

```sh
PHYSLIST=QGSP_BERT ./run.sh             # override physics list via env var
```

## What's in the example

| File | Description |
|------|-------------|
| `detector.tg` | Text geometry: HPGe crystal in Al housing |
| `run.sh` | Unified runner script |

## Detector

HPGe crystal (radius 35 mm, height 45 mm) in aluminum housing, in air.
Source: 2.6 MeV gamma, isotropic.

## Output columns

**Event mode** (`--mode event`):

| Column | Type | Description |
|--------|------|-------------|
| `n` | int | Step points recorded in this event |
| `m` | int | Highest copy number among sensitive volumes |
| `pdg0` | int | Source particle PDG (22 = gamma) |
| `k0` | double | Source kinetic energy [keV] |
| `etotal` | double | Total energy deposited [keV] |
| `et` | vector | Energy per sensitive volume [keV] |

**Step mode** (`--mode step`): all of the above plus 24 step-level vectors (trk, stp, vlm, pro, pdg, pid, xx, yy, zz, dt, de, dl, l, x, y, z, t, k, p, px, py, pz, q, et).

## Analysis

```python
import uproot, numpy as np
t = uproot.open("output.root")["t"]
etotal = t["etotal"].array(library="np")
print(f"Entries: {t.num_entries}, pdg0: {np.unique(t['pdg0'].array(library='np'))}")
```
