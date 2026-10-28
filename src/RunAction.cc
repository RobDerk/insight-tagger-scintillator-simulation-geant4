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
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

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

    analysisManager->CreateNtuple("PhotonHits", "Optical photons reaching top or bottom detector surfaces");

    // IDs should really be integer columns
    analysisManager->CreateNtupleIColumn("runID");
    analysisManager->CreateNtupleIColumn("eventID");
    analysisManager->CreateNtupleIColumn("trackID");
    analysisManager->CreateNtupleSColumn("hit");
    analysisManager->CreateNtupleDColumn("globaltime");
    analysisManager->CreateNtupleDColumn("timesinceelectronhit");
    analysisManager->CreateNtupleDColumn("trackLength");
    analysisManager->CreateNtupleDColumn("photonEnergy");
    analysisManager->CreateNtupleDColumn("photonFlightTime");
    analysisManager->CreateNtupleDColumn("photonCreationTime");
    analysisManager->CreateNtupleSColumn("creatorProcess");
    analysisManager->CreateNtupleIColumn("nSideReflections");
    analysisManager->CreateNtupleDColumn("scintLength");
    analysisManager->CreateNtupleDColumn("emissionAngle");

    analysisManager->FinishNtuple();
}

RunAction::~RunAction()
{

}

void RunAction::BeginOfRunAction(const G4Run* run)
{
    // inform the runManager to save random number seed
    fRunStart = std::chrono::steady_clock::now();

    G4RunManager::GetRunManager()->SetRandomNumberStore(false);

    // intialize the analysis manager to store data for photon hit top or bottom
    auto analysisManager = G4AnalysisManager::Instance();

    const G4int runID = run->GetRunID();

    const auto* detector = static_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    std::ostringstream nameStream;
    nameStream << detector->GetScintLengthY() / mm << "mm";

    fBaseName = nameStream.str();

    const G4String fileName = fBaseName + ".csv";

    G4cout
    << "\n====================================\n"
    << "Starting Run " << runID << "\n"
    << "Output base name: " << fileName << "\n"
    << "====================================\n"
    << G4endl;

    if (!analysisManager->OpenFile(fileName))
    {
        G4cerr
            << "ERROR: Could not open analysis file "
            << fileName
            << G4endl;
    }

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

    // Geant4 appends the ntuple name to the file name
    const std::filesystem::path generated =
        std::filesystem::path("data") / (fBaseName + "_PhotonHits.csv");

    if (std::filesystem::exists(generated))
    {
        std::filesystem::rename(
            generated,
            std::filesystem::path("data") / (fBaseName + ".csv")
        );
    }
    else
    {
        G4cerr << "WARNING: expected output file not found: " << generated << G4endl;
    }

    const G4int nEvents = run->GetNumberOfEvent();

    const G4double runTime =
        std::chrono::duration<G4double>(
            std::chrono::steady_clock::now() - fRunStart
        ).count();

    G4cout
        << "\n====================================\n"
        << "Run " << run->GetRunID() << " finished\n"
        << "  Events            : " << nEvents << "\n"
        << "  Total Edep        : " << G4BestUnit(edep, "Energy") << "\n"
        << "  Total sim. time   : " << runTime << " s\n";

    if (nEvents > 0)
    {
        G4cout
            << "  Time per event    : " << runTime / nEvents << " s\n";
    }

    G4cout
        << "===================================="
        << G4endl;
}

void RunAction::AddEdep(G4double edep)
{
    fEdep += edep;
    fEdep2 += edep * edep;
}

void RunAction::WritePhotonCounts(G4int eventID, G4int nTop, G4int nBottom) const
{

}