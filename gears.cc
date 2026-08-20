/**
 * \mainpage notitle
 * Homepage: <https://github.com/jintonic/gears>
 */
#include <set>
#include <vector>
using namespace std;
#include <G4SteppingManager.hh>
#include <G4SteppingVerbose.hh>
#include <G4UIcmdWithAString.hh>
#include <G4UImessenger.hh>
//______________________________________________________________________________
//
static set<G4String> sensitiveVolumes;
static G4String outputMode = "event";
static int maxStepsPerEvent = 10000;
//______________________________________________________________________________
//
class SensitiveMessenger : public G4UImessenger {
  G4UIcmdWithAString *fCmd;
public:
  SensitiveMessenger() {
    fCmd = new G4UIcmdWithAString("/sensitive/add", this);
    fCmd->SetGuidance("Mark a logical volume as sensitive");
    fCmd->SetParameterName("volume", false);
    fCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  }
  ~SensitiveMessenger() { delete fCmd; }
  void SetNewValue(G4UIcommand *, G4String value) {
    sensitiveVolumes.insert(value);
    G4cout << "GEARS: sensitive volume added: " << value << G4endl;
  }
};
//______________________________________________________________________________
//
class OutputMessenger : public G4UImessenger {
  G4UIcmdWithAString *fCmdMode;
  G4UIcommand *fCmdMax;
public:
  OutputMessenger() {
    fCmdMode = new G4UIcmdWithAString("/output/mode", this);
    fCmdMode->SetGuidance("Set output mode: event or step");
    fCmdMode->SetCandidates("event step");
    fCmdMode->AvailableForStates(G4State_PreInit);
    fCmdMax = new G4UIcommand("/output/maxSteps", this);
    fCmdMax->SetGuidance("Max step points per event (step mode only)");
    auto *p = new G4UIparameter("n", 'i', false);
    fCmdMax->SetParameter(p);
    fCmdMax->AvailableForStates(G4State_PreInit);
  }
  ~OutputMessenger() { delete fCmdMode; delete fCmdMax; }
  void SetNewValue(G4UIcommand *cmd, G4String value) {
    if (cmd == fCmdMode) outputMode = value;
    if (cmd == fCmdMax) maxStepsPerEvent = G4UIcommand::ConvertToInt(value);
  }
};
/**
 * Dump simulation results to screen or a file.
 */
class Output : public G4SteppingVerbose {
protected:
  void Record(); ///< Record simulated data
public:
  Output(); ///< use analysis manager to handle output
  void BookNtuple(); ///< create ntuple (called from BeginOfRunAction)
  void TrackingStarted() {
    G4SteppingVerbose::TrackingStarted();
    Record();
  } ///< Information of step 0 (initStep)
  void StepInfo() {
    G4SteppingVerbose::StepInfo();
    Record();
  } ///< Information of steps>0
  void Reset() {
    trk.clear();
    stp.clear();
    vlm.clear();
    pro.clear();
    pdg.clear();
    pid.clear();
    xx.clear();
    yy.clear();
    zz.clear();
    dt.clear();
    de.clear();
    dl.clear();
    l.clear();
    x.clear();
    y.clear();
    z.clear();
    t.clear();
    k.clear();
    p.clear();
    px.clear();
    py.clear();
    pz.clear();
    q.clear();
    et.clear();
    pdg0 = 0;
    k0 = 0;
    etotal = 0;
    stepOverflow = false;
  }
  void SetSteppingVerbose(int level) { fManager->SetVerboseLevel(level); }
  int GetSteppingVerbose() { return fManager->GetverboseLevel(); }

  int pdg0;           ///< source particle PDG encoding
  double k0;          ///< source particle kinetic energy [keV]
  double etotal;      ///< total energy deposit in sensitive volumes [keV]
  bool stepOverflow;  ///< true if step limit exceeded

  vector<int> trk;   ///< track ID
  vector<int> stp;   ///< step number
  vector<int> vlm;   ///< volume copy number
  vector<int> pro;   ///< process ID * 100 + sub-process ID
  vector<int> pdg;   ///< PDG encoding
  vector<int> pid;   ///< parent particle's PDG encoding
  vector<double> xx; ///< x [mm] (origin: center of local volume)
  vector<double> yy; ///< y [mm] (origin: center of local volume)
  vector<double> zz; ///< z [mm] (origin: center of local volume)
  vector<double> dt; ///< time elapsed from previous step point [ns]
  vector<double> de; ///< energy deposited [keV]
  vector<double> dl; ///< step length [mm]
  vector<double> l;  ///< length of track till this point [mm]
  vector<double> x;  ///< x [mm] (origin: center of the world)
  vector<double> y;  ///< y [mm] (origin: center of the world)
  vector<double> z;  ///< z [mm] (origin: center of the world)
  vector<double> t;  ///< time elapsed from the beginning of an event [ns]
  vector<double> k;  ///< kinetic energy [keV]
  vector<double> p;  ///< momentum [keV]
  vector<double> px; ///< x component of momentum direction
  vector<double> py; ///< y component of momentum direction
  vector<double> pz; ///< z component of momentum direction
  vector<double> q;  ///< charge [elementary charge]
  vector<double> et; ///< Total energy deposited in a volume [keV]
};
//______________________________________________________________________________
//
#include <G4AnalysisManager.hh>
Output::Output() : G4SteppingVerbose(), pdg0(0), k0(0), etotal(0),
    stepOverflow(false) {
  G4AnalysisManager::Instance(); // register /analysis/ UI commands
}
//______________________________________________________________________________
//
void Output::BookNtuple() {
  auto manager = G4AnalysisManager::Instance();
  if (outputMode == "event") {
    manager->CreateNtuple("t", "gears events");
    manager->CreateNtupleIColumn("n");
    manager->CreateNtupleIColumn("m");
    manager->CreateNtupleIColumn("pdg0");
    manager->CreateNtupleDColumn("k0");
    manager->CreateNtupleDColumn("etotal");
    manager->CreateNtupleDColumn("et", et);
  } else {
    manager->CreateNtuple("t", "gears step points");
    manager->CreateNtupleIColumn("n");
    manager->CreateNtupleIColumn("m");
    manager->CreateNtupleIColumn("trk", trk);
    manager->CreateNtupleIColumn("stp", stp);
    manager->CreateNtupleIColumn("vlm", vlm);
    manager->CreateNtupleIColumn("pro", pro);
    manager->CreateNtupleIColumn("pdg", pdg);
    manager->CreateNtupleIColumn("pid", pid);
    manager->CreateNtupleDColumn("xx", xx);
    manager->CreateNtupleDColumn("yy", yy);
    manager->CreateNtupleDColumn("zz", zz);
    manager->CreateNtupleDColumn("dt", dt);
    manager->CreateNtupleDColumn("de", de);
    manager->CreateNtupleDColumn("dl", dl);
    manager->CreateNtupleDColumn("l", l);
    manager->CreateNtupleDColumn("x", x);
    manager->CreateNtupleDColumn("y", y);
    manager->CreateNtupleDColumn("z", z);
    manager->CreateNtupleDColumn("t", t);
    manager->CreateNtupleDColumn("k", k);
    manager->CreateNtupleDColumn("p", p);
    manager->CreateNtupleDColumn("px", px);
    manager->CreateNtupleDColumn("py", py);
    manager->CreateNtupleDColumn("pz", pz);
    manager->CreateNtupleDColumn("q", q);
    manager->CreateNtupleDColumn("et", et);
    manager->CreateNtupleIColumn("pdg0");
    manager->CreateNtupleDColumn("k0");
    manager->CreateNtupleDColumn("etotal");
  }
  manager->FinishNtuple();
}
//______________________________________________________________________________
//
#include <G4NavigationHistory.hh>
void Output::Record() {
  if (GetSilent() == 1) // CopyState() won't be called in G4SteppingVerbose
    CopyState();        // point fTrack, fStep, etc. to right places

  G4TouchableHandle handle = fStep->GetPreStepPoint()->GetTouchableHandle();
  G4String volName = handle->GetVolume()->GetLogicalVolume()->GetName();
  if (sensitiveVolumes.find(volName) == sensitiveVolumes.end())
    return; // skip non-sensitive volumes

  if (outputMode == "step" && !stepOverflow) {
    if (trk.size() >= (size_t)maxStepsPerEvent) {
      stepOverflow = true;
    }
  }
  if (outputMode == "step" && stepOverflow)
    return; // stop recording steps but continue physics

  int copyNo = handle->GetReplicaNumber();
  trk.push_back(fTrack->GetTrackID());
  stp.push_back(fTrack->GetCurrentStepNumber());
  vlm.push_back(copyNo);
  pdg.push_back(fTrack->GetDefinition()->GetPDGEncoding());
  pid.push_back(fTrack->GetParentID());
  if (stp.back() == 0) { // step zero
    if (pid.back() != 0) // not primary particle
      pro.push_back(fTrack->GetCreatorProcess()->GetProcessType() * 1000 +
                    fTrack->GetCreatorProcess()->GetProcessSubType());
    else
      pro.push_back(1000); // primary particle
  } else {
    const G4VProcess *pr = fStep->GetPostStepPoint()->GetProcessDefinedStep();
    if (pr)
      pro.push_back(pr->GetProcessType() * 1000 + pr->GetProcessSubType());
    else
      pro.push_back(900); // not sure why pr can be zero
  }

  k.push_back(fTrack->GetKineticEnergy() / CLHEP::keV);
  p.push_back(fTrack->GetMomentum().mag() / CLHEP::keV);
  q.push_back(fStep->GetPostStepPoint()->GetCharge());
  l.push_back(fTrack->GetTrackLength() / CLHEP::mm);

  px.push_back(fTrack->GetMomentumDirection().x());
  py.push_back(fTrack->GetMomentumDirection().y());
  pz.push_back(fTrack->GetMomentumDirection().z());
  de.push_back(fStep->GetTotalEnergyDeposit() / CLHEP::keV);
  dl.push_back(fTrack->GetStepLength() / CLHEP::mm);

  t.push_back(fTrack->GetGlobalTime() / CLHEP::ns);
  x.push_back(fTrack->GetPosition().x() / CLHEP::mm);
  y.push_back(fTrack->GetPosition().y() / CLHEP::mm);
  z.push_back(fTrack->GetPosition().z() / CLHEP::mm);

  G4ThreeVector pos = handle->GetHistory()->GetTopTransform().TransformPoint(
      fStep->GetPostStepPoint()->GetPosition());
  xx.push_back(pos.x() / CLHEP::mm);
  yy.push_back(pos.y() / CLHEP::mm);
  zz.push_back(pos.z() / CLHEP::mm);
  dt.push_back(fTrack->GetLocalTime() / CLHEP::ns);

  if (de.back() > 0) {
    int copyNo = handle->GetReplicaNumber();
    if (et.size() < (unsigned int)copyNo + 1)
      et.resize((unsigned int)copyNo + 1);
    et[copyNo] += de.back();
    et[0] += de.back();
  }
}
//______________________________________________________________________________
//
#include <G4NistManager.hh>
#include <G4tgbVolumeMgr.hh>
//______________________________________________________________________________
//
#include <G4tgbDetectorBuilder.hh>
/**
 * Construct detector based on text geometry description.
 */
class TextDetectorBuilder : public G4tgbDetectorBuilder {
public:
  TextDetectorBuilder() : G4tgbDetectorBuilder() {}
};
//______________________________________________________________________________
//
#include <G4UIcmdWith3VectorAndUnit.hh>
#include <G4UIdirectory.hh>
#include <G4VUserDetectorConstruction.hh>
/**
 * Construct detector geometry.
 *
 * This uses two types of instructions to construct a detector:
 *
 * - [Geant4 text
 * geometry](http://geant4-userdoc.web.cern.ch/geant4-userdoc/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomASCII.html)
 * -
 * [GDML](http://geant4-userdoc.web.cern.ch/geant4-userdoc/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomXML.html)
 *
 * It won't work together with HP neutron simulation if Geant4 version is lower
 * than 10 because of this bug:
 * http://hypernews.slac.stanford.edu/HyperNews/geant4/get/hadronprocess/1242.html?inline=-1
 */
class Detector : public G4VUserDetectorConstruction, public G4UImessenger {
public:
  Detector();
  ~Detector() {
    delete fCmdSetB;
    delete fCmdSrc;
    delete fCmdOut;
  }
  G4VPhysicalVolume *Construct(); ///< called at /run/initialize
  void SetNewValue(G4UIcommand *cmd, G4String value); ///< for G4UI

private:
  G4UIcmdWith3VectorAndUnit *fCmdSetB; ///< /geometry/setB
  G4UIcmdWithAString *fCmdSrc;         ///< /geometry/source
  G4UIcmdWithAString *fCmdOut;         ///< /geometry/export
  G4UIcommand *fCmdLmt;                ///< /tracking/setStepLimit
  G4VPhysicalVolume *fWorld;
};
//______________________________________________________________________________
//
Detector::Detector()
    : G4VUserDetectorConstruction(), G4UImessenger(), fWorld(0) {
#ifdef hasGDML
  fCmdOut = new G4UIcmdWithAString("/geometry/export", this);
  fCmdOut->SetGuidance("Export geometry gdml file name");
  fCmdOut->SetParameterName("gdml geometry output", false);
  fCmdOut->AvailableForStates(G4State_Idle);
#else
  fCmdOut = 0;
#endif

  fCmdSrc = new G4UIcmdWithAString("/geometry/source", this);
  fCmdSrc->SetGuidance("Set geometry source file name");
  fCmdSrc->SetParameterName("text geometry input", false);
  fCmdSrc->AvailableForStates(G4State_PreInit);

  fCmdLmt = new G4UIcommand("/tracking/setStepLimit", this);
  fCmdLmt->SetGuidance("set max step length for a logical volume");
  fCmdLmt->AvailableForStates(G4State_Idle);
  G4UIparameter *p0 = new G4UIparameter("logical volume name", 's', false);
  G4UIparameter *p1 = new G4UIparameter("step length in mm", 'd', false);
  fCmdLmt->SetParameter(p0);
  fCmdLmt->SetParameter(p1);

  fCmdSetB = new G4UIcmdWith3VectorAndUnit("/geometry/SetB", this);
  fCmdSetB->SetGuidance("Set uniform magnetic field value.");
  fCmdSetB->SetParameterName("Bx", "By", "Bz", false);
  fCmdSetB->SetUnitCategory("Magnetic flux density");
}
//______________________________________________________________________________
//
#include <G4FieldManager.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4TransportationManager.hh>
#include <G4UniformMagField.hh>
#include <G4UserLimits.hh>
#ifdef hasGDML
#include "G4GDMLParser.hh"
#endif
void Detector::SetNewValue(G4UIcommand *cmd, G4String value) {
  if (cmd == fCmdSetB) {
    auto field = new G4UniformMagField(0, 0, 0);
    field->SetFieldValue(fCmdSetB->GetNew3VectorValue(value));
    G4FieldManager *mgr =
        G4TransportationManager::GetTransportationManager()->GetFieldManager();
    mgr->SetDetectorField(field);
    mgr->CreateChordFinder(field);
    G4cout << "GEARS: Magnetic field is set to " << value << G4endl;
#ifdef hasGDML
  } else if (cmd == fCmdOut) {
    G4GDMLParser paser;
    paser.Write(value, fWorld);
#endif
  } else if (cmd == fCmdLmt) {
    istringstream iss(value);
    G4String vlm, limit;
    iss >> vlm >> limit;
    auto v = G4LogicalVolumeStore::GetInstance()->GetVolume(vlm);
    if (v) {
      v->SetUserLimits(new G4UserLimits(stof(limit)));
      G4cout << "GEARS: max step length in " << vlm << ": " << limit << " mm"
             << G4endl;
    }
  } else {                                            // cmd==fCmdSrc
    if (value.substr(value.length() - 4) != "gdml") { // text geometry input
      G4tgbVolumeMgr *mgr = G4tgbVolumeMgr::GetInstance();
      mgr->AddTextFile(value);
      auto tgb = new TextDetectorBuilder;
      mgr->SetDetectorBuilder(tgb);
      fWorld = mgr->ReadAndConstructDetector();
#ifdef hasGDML
    } else { // GDML input
      G4GDMLParser parser;
      parser.Read(value);
      fWorld = parser.GetWorldVolume();
#endif
    }
  }
}
//______________________________________________________________________________
//
#include "G4Box.hh"
#include "G4PVPlacement.hh"
G4VPhysicalVolume *Detector::Construct() {
  if (fWorld == NULL) {
    G4cout << "GEARS: no detector specified, set to a 10x10x10 m^3 box."
           << G4endl;
    auto box = new G4Box("hall", 5 * CLHEP::m, 5 * CLHEP::m, 5 * CLHEP::m);
    G4NistManager *nist = G4NistManager::Instance();
    G4Material *vacuum = nist->FindOrBuildMaterial("G4_Galactic");
    auto v = new G4LogicalVolume(box, vacuum, "hall");
    fWorld = new G4PVPlacement(0, G4ThreeVector(), v, "hall", 0, 0, 0);
  }
  return fWorld;
}
//______________________________________________________________________________
//
#include <G4GeneralParticleSource.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
/**
 * Call Geant4 General Particle Source to generate particles.
 */
class Generator : public G4VUserPrimaryGeneratorAction {
private:
  G4GeneralParticleSource *fSource;

public:
  Generator() : G4VUserPrimaryGeneratorAction(), fSource(0) {
    fSource = new G4GeneralParticleSource;
  }
  virtual ~Generator() { delete fSource; }
  virtual void GeneratePrimaries(G4Event *evt) {
    fSource->GeneratePrimaryVertex(evt);
  } ///< add sources to an event
};
//______________________________________________________________________________
//
#include <G4Run.hh>
#include <G4RunManagerFactory.hh>
#include <G4UserRunAction.hh>
/**
 * Book keeping before and after a run.
 */
class RunAction : public G4UserRunAction {
public:
  void BeginOfRunAction(const G4Run *) {
    auto a = G4AnalysisManager::Instance();
    if (a->GetFileName() == "")
      return;
    Output *o = ((Output *)G4VSteppingVerbose::GetInstance());
    o->BookNtuple();
    a->OpenFile();
    if (o->GetSteppingVerbose() == 0) { // in case of /tracking/verbose 0
      o->SetSilent(1);                  // avoid screen dump
      o->SetSteppingVerbose(1);         // enable calling StepInfo() in
                                // G4SteppingManager
    }
  } ///< enable output if output file name is not empty
  void EndOfRunAction(const G4Run *) {
    auto a = G4AnalysisManager::Instance();
    if (a->GetFileName() != "") {
      a->Write();
      a->CloseFile();
    }
  } ///< Close output file
};
//______________________________________________________________________________
//
#include <G4Event.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <G4UserEventAction.hh>
/**
 * Book keeping before and after an event.
 */
class EventAction : public G4UserEventAction {
public:
  void BeginOfEventAction(const G4Event *evt) {
    Output *o = ((Output *)G4VSteppingVerbose::GetInstance());
    o->pdg0 = 0;
    o->k0 = 0;
    o->etotal = 0;
    o->stepOverflow = false;
    auto *vtx = evt->GetPrimaryVertex(0);
    if (vtx) {
      auto *p = vtx->GetPrimary(0);
      if (p) {
        o->pdg0 = p->GetPDGcode();
        o->k0 = p->GetKineticEnergy() / CLHEP::keV;
      }
    }
  }
  void EndOfEventAction(const G4Event *) {
    auto a = G4AnalysisManager::Instance();
    Output *o = ((Output *)G4VSteppingVerbose::GetInstance());
    if (a->GetFileName() == "") {
      o->Reset();
      return;
    }
    o->etotal = o->et.empty() ? 0 : o->et[0];
    if (outputMode == "event") {
      a->FillNtupleIColumn(0, o->stp.size());
      a->FillNtupleIColumn(1, o->et.size() - 1);
      a->FillNtupleIColumn(2, o->pdg0);
      a->FillNtupleDColumn(3, o->k0);
      a->FillNtupleDColumn(4, o->etotal);
      a->AddNtupleRow();
    } else {
      if (!o->stp.empty()) {
        a->FillNtupleIColumn(0, o->stp.size());
        a->FillNtupleIColumn(1, o->et.size() - 1);
        // columns 2-25 are vector branches (auto-filled)
        a->FillNtupleIColumn(26, o->pdg0);
        a->FillNtupleDColumn(27, o->k0);
        a->FillNtupleDColumn(28, o->etotal);
        a->AddNtupleRow();
      }
    }
    o->Reset();
  }
};
//______________________________________________________________________________
//
#include <G4UIcmdWithADoubleAndUnit.hh>
#include <G4UserStackingAction.hh>
/**
 * Split a radioactive decay chain to different events based on a time window.
 */
class StackingAction : public G4UserStackingAction, public G4UImessenger {
private:
  double fT0;         ///< reference time for splitting decay chain
  double fTimeWindow; ///< time window to split a decay chain
  G4UIcmdWithADoubleAndUnit *fCmdT; ///< UI cmd to set time window
public:
  StackingAction()
      : G4UserStackingAction(), G4UImessenger(), fT0(0), fTimeWindow(0),
        fCmdT(0) {
    fCmdT = new G4UIcmdWithADoubleAndUnit("/process/had/rdm/setTimeWindow", this);
    fCmdT->SetGuidance("Time window to split a radioactive decay chain.");
    fCmdT->SetGuidance("If a daughter nucleus appears after the window,");
    fCmdT->SetGuidance("it is saved in a new entry in the output ntuple.");
    fCmdT->SetGuidance("---Set it to <=0 to disable the splitting---");
    fCmdT->SetParameterName("time window", false, true);
    fCmdT->SetDefaultUnit("s");
    fCmdT->AvailableForStates(G4State_PreInit, G4State_Idle);
  } ///< created macro /process/had/rdm/setTimeWindow
  ~StackingAction() { delete fCmdT; }
  G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track *trk) {
    if (fTimeWindow <= 0)
      return fUrgent; // no need to split
    if (trk->GetGlobalTime() > fT0 + fTimeWindow)
      return fWaiting; // split
    else
      return fUrgent; // too fast to be split
  } ///< send a daughter particle to waiting stack if it appears too late
  void NewStage() { // called after processing urgent trk, before waiting trk
    if (fTimeWindow <= 0)
      return; // do nothing if no time window is specified
    Output *o = ((Output *)G4VSteppingVerbose::GetInstance());
    fT0 = o->t.back();   // update the reference time to the latest decay time
    auto a = G4AnalysisManager::Instance();
    if (a->GetFileName() != "" && !o->stp.empty()) {
      o->etotal = o->et.empty() ? 0 : o->et[0];
      a->FillNtupleIColumn(0, o->stp.size());
      a->FillNtupleIColumn(1, o->et.size() - 1);
      a->AddNtupleRow();
    }
    o->Reset();
  } ///< save and reset output before processing waiting tracks
  void SetNewValue(G4UIcommand *cmd, G4String value) {
    if (cmd != fCmdT)
      return;
    fTimeWindow = fCmdT->GetNewDoubleValue(value);
  }
};
//______________________________________________________________________________
//
#include <G4VUserActionInitialization.hh>
class Action : public G4VUserActionInitialization {
  void Build() const {
    SetUserAction(new RunAction);
    SetUserAction(new Generator);
    SetUserAction(new EventAction);
    SetUserAction(new StackingAction);
  }
};
//______________________________________________________________________________
//
#include <G4PhysListFactory.hh>
#include <G4ScoringManager.hh>
#include <G4StepLimiterPhysics.hh>
#include <G4UIExecutive.hh>
#include <G4UImanager.hh> // needed for g4.10 and above
#include <G4VisExecutive.hh>
/**
 * The main function that calls individual components.
 */
int main(int argc, char **argv) {
  auto *sensitiveMessenger = new SensitiveMessenger;
  auto *outputMessenger = new OutputMessenger;
  // inherit G4SteppingVerbose instead of G4UserSteppingAction to record data
  G4VSteppingVerbose::SetInstance(new Output); // must be before run manager
  auto run =
      G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);
  G4PhysListFactory factory;
  auto physics = factory.ReferencePhysList();
  physics->RegisterPhysics(new G4StepLimiterPhysics());
  run->SetUserInitialization(physics);      // initialize physics
  run->SetUserInitialization(new Detector); // initialize detector
  run->SetUserInitialization(new Action);   // initialize user actions
  G4ScoringManager::GetScoringManager();    // enable built-in scoring cmds
  G4UIExecutive *ui = nullptr;              // assume batch mode
  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }                                       // interactive mode
  auto vis = new G4VisExecutive("quiet"); // visualization
  vis->Initialize();                      // do this after ui mode is decided
  if (ui) {                               // interactive mode
    ui->SessionStart();                   // do this after vis
    delete ui;
  } else { // batch mode
    G4String cmd = "/control/execute ";
    G4UImanager::GetUIpointer()->ApplyCommand(cmd + argv[1]);
  }
  delete vis;
  delete run;
  delete outputMessenger;
  delete sensitiveMessenger;
  return 0;
}
// -*- C++; indent-tabs-mode:nil; tab-width:2 -*-
// vim: ft=cpp:ts=2:sts=2:sw=2:et
