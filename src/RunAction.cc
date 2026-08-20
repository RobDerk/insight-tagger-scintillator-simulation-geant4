#include "RunAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4AccumulableManager.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4AnalysisManager.hh"

RunAction::RunAction()
{
    // Register accumulable to the accumulable manager
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(fEdep);
    accumulableManager->Register(fEdep2);
}

RunAction::~RunAction()
{

}

void RunAction::BeginOfRunAction(const G4Run* run)
{
    // inform the runManager to save random number seed
    G4RunManager::GetRunManager()->SetRandomNumberStore(false);

    // // intialize the analysis manager
    // auto analysisManager = G4AnalysisManager::Instance();

    // // storing data in csv file
    // std::string runNumber = std::to_string(run->GetRunID());
    // G4String fileName = "run_" + runNumber + ".csv";

    // analysisManager->SetNtupleDirectoryName("data");
    // analysisManager->OpenFile(fileName);

    // // Create an ntuple (row-based columns)
    // analysisManager->CreateNtuple("StepData", "Energy Deposit and Step Info");
    // analysisManager->CreateNtupleDColumn("eventID");
    // analysisManager->CreateNtupleDColumn("edep");
    // analysisManager->CreateNtupleDColumn("ekin");
    // analysisManager->CreateNtupleDColumn("posX");
    // analysisManager->CreateNtupleDColumn("posY");
    // analysisManager->CreateNtupleDColumn("posZ");
    // analysisManager->CreateNtupleDColumn("momX");
    // analysisManager->CreateNtupleDColumn("momY");
    // analysisManager->CreateNtupleDColumn("momZ");
    // analysisManager->CreateNtupleDColumn("stepLen");
    // analysisManager->CreateNtupleDColumn("stepLx");
    // analysisManager->CreateNtupleDColumn("stepLy");
    // analysisManager->CreateNtupleDColumn("stepLz");
    // analysisManager->CreateNtupleDColumn("tof");
    // analysisManager->CreateNtupleDColumn("globalTime");
    // analysisManager->CreateNtupleDColumn("trackID");
    // analysisManager->CreateNtupleDColumn("parentID");
    // analysisManager->CreateNtupleSColumn("particleName");
    // analysisManager->FinishNtuple();

    // analysisManager->AddNtupleRow();

    // intialize the analysis manager to store data for photon hit top or bottom
    auto analysisManager = G4AnalysisManager::Instance();

    // storing data in csv file
    std::string runNumber = std::to_string(run->GetRunID());
    G4String fileName = "photon_run_" + runNumber + ".csv";

    analysisManager->SetNtupleDirectoryName("data");
    analysisManager->OpenFile(fileName);

    // Create an ntuple (row-based columns)
    analysisManager->CreateNtuple("StepData", "Energy Deposit and Step Info");
    analysisManager->CreateNtupleDColumn("eventID");
    analysisManager->CreateNtupleSColumn("hit");
    analysisManager->CreateNtupleDColumn("globaltime");
    analysisManager->CreateNtupleDColumn("localtime");
    analysisManager->FinishNtuple();

    analysisManager->AddNtupleRow();

    // reset accumulables to their initial values
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4int nofEvents = run->GetNumberOfEvent();
    if (nofEvents == 0) return;

    // Merge accumulables
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Merge();

    // Compute dose = total energy deposit in a run and its variance
    G4double edep = fEdep.GetValue();
    G4double edep2 = fEdep2.GetValue();

    const auto detConstruction = static_cast<const DetectorConstruction*>(G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    G4double mass = detConstruction->GetScoringVolume()->GetMass();

    // Run conditions
    // note: There is no primary generator action object for "master" run manager for multi-threaded mode.
    const auto generatorAction = static_cast<const PrimaryGeneratorAction*>(G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
    G4String runCondition;

    if (generatorAction) {
        const G4ParticleGun* particleGun = generatorAction->GetParticleGun();
        runCondition += particleGun->GetParticleDefinition()->GetParticleName();
        runCondition += " of ";
        G4double particleEnergy = particleGun->GetParticleEnergy();
        runCondition += G4BestUnit(particleEnergy, "Energy");
    }

    // stroing data in csv
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();

    // // Print
    // if (IsMaster()) {
    //     G4cout << G4endl << "--------------------End of Global Run-----------------------";
    // }
    // else {
    //     G4cout << G4endl << "--------------------End of Local Run------------------------";
    // }

    // G4cout << G4endl << " The run is " << nofEvents << " " << runCondition << G4endl << G4endl;
    // G4cout << "  --> cumulated edep per run in scoring volume = " << G4BestUnit(edep, "Energy") 
    //         << " = " << edep/joule << " joule" << G4endl;  
    // G4cout << "  --> mass of scoring volume = " << G4BestUnit(mass, "Mass") << G4endl << G4endl; 
    // G4cout << "------------------------------------------------------------" << G4endl << G4endl;
}

void RunAction::AddEdep(G4double edep)
{
    fEdep += edep;
    fEdep2 += edep * edep;
}

void RunAction::WritePhotonCounts(G4int eventID, G4int nTop, G4int nBottom) const
{

}