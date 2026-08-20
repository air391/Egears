[![YouTube](https://img.shields.io/badge/You-Tube-red?style=flat)](https://youtu.be/E7VpAcXhhHo)
[![alpha](https://img.shields.io/badge/alpha-interactions-blue?style=flat)](alpha)
[![beta](https://img.shields.io/badge/beta-interactions-yellow?style=flat)](beta)
[![gamma](https://img.shields.io/badge/gamma-interactions-cyan?style=flat)](gamma)
[![muon](https://img.shields.io/badge/muon-interactions-pink?style=flat)](muon)
[![neutron](https://img.shields.io/badge/neutron-interactions-brown?style=flat)](neutron)
[![X-ray](https://img.shields.io/badge/X--ray-creation-green?style=flat)](X-ray)

## Terminology

1. [physics model][physics list]: what happens in a certain energy range
2. [physics process][physics list]: model + cross section (how often it happens)
3. [physics list][]: a list of processes for common particles
4. [modular lists][]: lists of processes that can be used as building blocks to construct a more complex list
5. [reference lists][]: official [modular lists][] shipped with [Geant4][]
6. [factory][]: a [Geant4][] class that can be used to call [reference lists][] by their names

[physics list]: http://geant4-userdoc.web.cern.ch/geant4-userdoc/UsersGuides/ForApplicationDeveloper/html/UserActions/mandatoryActions.html#physics-lists
[modular lists]: http://geant4-userdoc.web.cern.ch/geant4-userdoc/UsersGuides/ForApplicationDeveloper/html/UserActions/mandatoryActions.html#building-physics-list-from-physics-builders
[reference lists]: https://geant4.web.cern.ch/documentation/dev/plg_html/PhysicsListGuide/physicslistguide.html
[factory]: https://geant4.kek.jp/lxr/source/physics_lists/lists/src/G4PhysListFactory.cc#L82
[Geant4]: http://geant4.cern.ch

## How to choose a physics list

[GEARS][] uses [G4PhysListFactory.cc][factory] to allow the user to select one from the [reference lists][] using an environment variable `PHYSLIST` before the `gears` executable:

```sh
$ PHYSLIST=FTFP_BERT_EMZ gears
```

Available reference lists can be found in [G4PhysListFactory.cc][factory]. A guidance on how to choose a proper physics list is available in this [tutorial][].

[GEARS]: http://physino.xyz/gears
[tutorial]: https://www.slac.stanford.edu/xorg/geant4/SLACTutorial14/Physics1.pdf

## Physics processes

Major categories of processes provided in [Geant4][]:

- Electromagnetism (EM)
  - Standard processes (~1 keV to ~PeV)
  - Low energy processes (250 eV to ~PeV)
- Weak interaction
  - decay of subatomic particles
  - radioactive decay of nuclei
- Hadronic physics
  - pure strong interaction (0 to ~TeV)
  - electro- and gamma-nuclear (10 MeV to ~TeV)
- Parameterized physics (not from first principles) for fast simulation
- Transportation (change from one volume to another)

Run `/process/list` after `/run/initialize` to see all active processes.

### Radioactive decay

Radioactive decay processes can be enabled before `/run/initialize`:

```sh
PreInit> /physics_lists/factory/addRadioactiveDecay
PreInit> /run/initialize
```

Detailed control of radioactive decay is provided by the [/process/had/rdm][] command:

```sh
/process/had/rdm/deselectVolume chamber
/process/had/rdm/nucleusLimits 1 80
/process/had/rdm/thresholdForVeryLongDecayTime 1e60 year
```

The last command is needed for Geant4 versions >=11.2, where its default value is set to 1 year. Any isotope whose lifetime is longer than 1 year will not decay without setting this to a very long time. This is documented in Geant4 [Physics List Guide].

[/process/had/rdm]: https://github.com/Geant4/geant4/blob/master/source/processes/hadronic/models/radioactive_decay/src/G4RadioactiveDecayMessenger.cc
[Physics List Guide]: https://geant4.web.cern.ch/documentation/dev/plg_html/PhysicsListGuide/hadronic/ui-commands.html

Example: create Pb210 on the surface of a cylindrical CsI detector:

```sh
/gps/particle ion
/gps/ion 82 210
/gps/energy 0

/gps/pos/type Surface
/gps/pos/shape Cylinder
/gps/pos/radius 7 cm
/gps/pos/halfz 2.5 cm
```

#### Stop decay chain

If the half life of a daughter nucleus is longer than a measurement duration, stop its radioactive decay completely:

```sh
/process/had/rdm/nucleusLimits 241 241 95 95
```

This enables radioactive decay only for Z=95, A=241 (Am-241). Daughter nuclei outside this range will not decay.
