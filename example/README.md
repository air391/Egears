# GEARS Example

## Build

```sh
nix develop
cmake -B build -S . && cmake --build build
```

## Run

```sh
cd example

# event mode (6 columns: n, m, pdg0, k0, etotal, et)
../build/gears event.mac

# step mode (29 columns: full step-point data)
../build/gears step.mac

# GDML geometry
../build/gears gen_gdml.mac        # generate detector.gdml from detector.tg
../build/gears gdml.mac            # run with GDML

# visualization (requires display server)
../build/gears vis.mac

# parallel (different seeds, merge with hadd)
../build/gears event.mac           # run in terminal 1
# edit event.mac: change /random/setSeeds and /analysis/setFileName
../build/gears event.mac           # run in terminal 2
hadd output.root output_*.root     # merge
```

## Physics list

```sh
PHYSLIST=QGSP_BERT ../build/gears event.mac
```

## What's here

| File | Description |
|------|-------------|
| `detector.tg` | Text geometry: HPGe crystal in Al housing |
| `event.mac` | Event mode run |
| `step.mac` | Step mode run |
| `gdml.mac` | GDML geometry run |
| `gen_gdml.mac` | Generate GDML from .tg |
| `vis.mac` | Qt visualization |

## Output

**Event mode** (`event.mac`): n, m, pdg0, k0, etotal, et

**Step mode** (`step.mac`): all of the above + trk, stp, vlm, pro, pdg, pid, xx, yy, zz, dt, de, dl, l, x, y, z, t, k, p, px, py, pz, q

## Analysis

```python
import uproot, numpy as np
t = uproot.open("output_event.root")["t"]
print(t.keys())
```
