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

#include <filesystem>

RunAction::RunAction()
{
    // Register accumulable to the accumulable manager
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();

    accumulableManager->Register(fEdep);
    accumulableManager->Register(fEdep2);

    // data to csv for Analysis
    std::filesystem::create_directories("data");

    auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->SetNtupleDirectoryName("data");

    analysisManager->CreateNtuple("StepData", "Optical photon timing data");

    // IDs should really be integer columns
    analysisManager->CreateNtupleIColumn("eventID");          // 0
    analysisManager->CreateNtupleIColumn("trackID");          // 1
    analysisManager->CreateNtupleSColumn("hit");              // 2
    analysisManager->CreateNtupleDColumn("globaltime");       // 3
    analysisManager->CreateNtupleDColumn("timesinceelectronhit"); // 4
    analysisManager->CreateNtupleDColumn("trackLength");      // 5
    analysisManager->CreateNtupleDColumn("photonEnergy");     // 6
    analysisManager->CreateNtupleDColumn("photonFlightTime"); // 7

    analysisManager->FinishNtuple();
}

RunAction::~RunAction()
{

}

void RunAction::BeginOfRunAction(const G4Run* run)
{
    // inform the runManager to save random number seed
    G4RunManager::GetRunManager()->SetRandomNumberStore(false);

    // intialize the analysis manager to store data for photon hit top or bottom
    auto analysisManager = G4AnalysisManager::Instance();

    // store data in a csv file
    const G4String fileName = "photon_run_" + std::to_string(run->GetRunID()) + ".csv";
    analysisManager->OpenFile(fileName);

    // reset accumulables to their initial values
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    // Merge accumulables
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Merge();

    // Compute dose = total energy deposit in a run and its variance
    G4double edep = fEdep.GetValue();
    G4double edep2 = fEdep2.GetValue();

    // stroing data in csv
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
}

void RunAction::AddEdep(G4double edep)
{
    fEdep += edep;
    fEdep2 += edep * edep;
}

void RunAction::WritePhotonCounts(G4int eventID, G4int nTop, G4int nBottom) const
{

}