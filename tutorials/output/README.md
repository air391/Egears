## Output

GEARS provides two output modes, selectable via macro command:

```
/output/mode event   # event-level summary (6 columns, default)
/output/mode step    # full step-point data (29 columns)
```

The output file name is set with:

```
/analysis/setFileName gears.root
```

Output is disabled by default — it is enabled only when the file name is not empty.

### Sensitive volumes

Before `/run/initialize`, mark which logical volumes should be recorded:

```
/sensitive/add HPGe
/sensitive/add chamber
```

Only steps inside sensitive volumes are recorded. Volumes not marked as sensitive are ignored.

### Event mode (default)

Each entry in the ROOT tree corresponds to one simulated event. Columns:

| Branch | Type | Description |
|--------|------|-------------|
| `n` | int | Number of step points recorded in this event |
| `m` | int | Highest copy number among sensitive volumes |
| `pdg0` | int | PDG encoding of the source particle |
| `k0` | double | Kinetic energy of the source particle [keV] |
| `etotal` | double | Total energy deposited in all sensitive volumes [keV] |
| `et` | vector\<double\> | Energy deposited per sensitive volume (indexed by copy number) |

### Step mode

Each entry contains the full step-point data from the event, plus the same event-level fields as event mode. The 29 branches are:

- Event-level: `n`, `m`, `pdg0`, `k0`, `etotal`
- Step-level vectors: `trk`, `stp`, `vlm`, `pro`, `pdg`, `pid`, `xx`, `yy`, `zz`, `dt`, `de`, `dl`, `l`, `x`, `y`, `z`, `t`, `k`, `p`, `px`, `py`, `pz`, `q`, `et`

### Step limit

In step mode, the maximum number of recorded step points per event can be set with:

```
/output/maxSteps 50000
```

Default is 10000. When the limit is reached, recording stops but physics continues — the event is tagged (not killed). This ensures event-level results remain correct.

### ROOT

The [ROOT][] version of ntuples is [TTree][]. Each row is an entry (event), each column is a branch. If you simulate 1000 events, the tree has 1000 entries. In event mode, each entry has 6 branches. In step mode, each entry has 29 branches, and vectors may contain multiple values per event (one per step point), creating a jagged array.

[TTree]: https://root.cern.ch/doc/master/classTTree.html

Example ROOT session:

```sh
$ root gears.root
root [] t->GetEntries()
(long long) 50000
root [] t->Show(0)
====> EVENT:0
 n      = 4
 m      = 1
 pdg0   = 22
 k0     = 2600
 etotal = 45.3
 et     = (vector<double>*)0x...
root [] t->Draw("etotal")          // energy spectrum
root [] t->Draw("k0")              // source energy
root [] t->Draw("pdg0")            // source particle type
```

In step mode:

```cpp
root [] t->Draw("x:y", "trk==1","l", 1, 1)   // primary particle tracks
root [] t->Draw("pro","trk>1 && stp==0")      // processes creating secondaries
root [] t->Draw("x:y:z", "vlm==1")            // hits in volume 1
root [] t->Draw("pdg")                         // particle types
root [] t->Draw("pro", "pdg==22 && stp!=0")    // gamma processes
root [] t->Draw("de/dl:p")                     // dE/dx vs momentum
root [] t->Draw("et[1]")                       // energy in volume 1
```

### Python

GEARS output can be analyzed in [Python][] using [uproot][]:

```python
import uproot, awkward as ak, numpy as np
import matplotlib.pyplot as plt

f = uproot.open("gears.root")
t = f["t"]

# event mode: energy spectrum
etotal = t["etotal"].array(library="np")
plt.hist(etotal[etotal > 0], bins=100)
plt.xlabel("Total energy deposited [keV]")
plt.show()

# step mode: particle positions
x = np.asarray(ak.flatten(t["x"].array()))
y = np.asarray(ak.flatten(t["y"].array()))
plt.hist2d(x, y, bins=100)
plt.show()
```

[Python]: https://www.python.org/
[uproot]: https://pypi.org/project/uproot/
[ROOT]: https://root.cern.ch

### Process id

The physics process generating each step point is saved in `pro[i]`, equal to (process type) \* 1000 + (sub type). Key values:

- 1000: initial step (step 0)
- 1092: coupled transportation
- 2002: ionization
- 2003: Bremsstrahlung
- 2010: multiple scattering
- 2012: photoelectric effect
- 2013: Compton scattering
- 2014: gamma conversion
- 4111: hadron elastic
- 4121: hadron inelastic
- 4131: neutron capture
- 4210: radioactive decay
- 6210: radioactive decay

### Particle id

`pdg` is the [PDG encoding](http://pdg.lbl.gov/current/mc-particle-id) of the particle. Common values: 22 (gamma), 11 (e-), -11 (e+), 2212 (proton), 2112 (neutron), 13 (mu-), -13 (mu+).

`pdg0` (event-level) is the PDG encoding of the source particle.

### Combine step points to hits

Many step points are very close together. A ROOT script [combineStepPointsToHits.C](combineStepPointsToHits.C) combines nearby step points into detector hits, saving to `hits.root`. [drawHits.C](drawHits.C) visualizes the result.

```sh
$ root -q combineStepPointsToHits.C
$ root drawHits.C
```

Run after `gears radiate.mac` to use its output as input.
