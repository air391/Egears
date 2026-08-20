## Tutorials

### Getting started

Run each demo separately:

```sh
gears demo_event.mac   # event mode (6 columns, 1 MB)
gears demo_step.mac    # step mode (29 columns, 44 MB)
gears demo_score.mac   # mesh scoring (CSV, 31 KB)
gears radiate.mac      # classic demo with visualization
```

- [**demo_event.mac**](output/demo_event.mac) — event mode: `/sensitive/add`, `/output/mode event`, source particle info (pdg0, k0), energy deposition (etotal, et)
- [**demo_step.mac**](output/demo_step.mac) — step mode: `/output/mode step`, `/output/maxSteps`, full step-point vectors
- [**demo_score.mac**](output/demo_score.mac) — mesh scoring: `/score/create/boxMesh`, `/score/quantity/energyDeposit`, `/score/dumpQuantityToFile`
- [**radiate.mac**](output/radiate.mac) — classic demo: VRML visualization + ROOT output

### New features

- `/sensitive/add <volume>` — mark logical volumes as sensitive (replaces the old `(S)` naming convention)
- `/output/mode event|step` — event mode: 6 columns (n, m, pdg0, k0, etotal, et); step mode: 29 columns (full step-point data)
- `/output/maxSteps N` — configurable step limit (step mode only); overflow tags the event without killing the track
- Event-level fields: `pdg0` (source PDG), `k0` (source kinetic energy), `etotal` (total energy deposit)

### Geometry

- [Detector construction](detector) — CSG, [boolean operations](detector/boolean), [scripts](detector/scripts), [GDML export](detector/GDML)
- [Visualization](detector/visualization) — OpenGL, VRML, DAWN, HepRep, ToolsSG, RayTracer, VTK

### Physics

- [Alpha](physics/alpha) — Bragg curve, range in air and CsI
- [Beta](physics/beta) — electron interactions in air
- [Gamma](physics/gamma) — photoelectric, Compton, pair production in air and lead
- [Muon](physics/muon) — cosmic ray muons, scintillating panels
- [Neutron](physics/neutron) — DT neutrons in concrete, Gd thermal capture, elastic scattering
- [X-ray](physics/X-ray) — PIXE fluorescence

### Sources

- [GPS source](sources) — Am-241, Co-57, Fe-55, Ar-39, cosmic muons
- [GPS manual](sources/gps.txt)

### Output

- [Output](output) — ROOT ntuple, scoring mesh, step length limits
- [Batch](batch) — SGE cluster farming (sge.sh)
- [UI](UI) — interactive and batch modes
