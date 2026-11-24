#include "RunAction.hh"

#include "CsvHitWriter.hh"
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

namespace
{
    // "1GeV", "500MeV", "1.5GeV"
    std::string EnergyLabel(G4double energy)
    {
        std::ostringstream label;

        if (energy >= GeV) label << energy / GeV << "GeV";
        else if (energy >= MeV) label << energy / MeV << "MeV";
        else label << energy / keV << "keV";

        return label.str();
    }
}

RunAction::RunAction(G4bool writeOutput) : fWriteOutput(writeOutput)
{
    // Register accumulable to the accumulable manager
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();

    accumulableManager->Register(fEdep);
    accumulableManager->Register(fEdep2);

    if (!fWriteOutput)
    {
        return;
    }

    // Hits go to a ROOT ntuple (analysis manager) and to a CSV file (CsvHitWriter)
    auto analysisManager = G4AnalysisManager::Instance();

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
    analysisManager->CreateNtupleSColumn("material");
    analysisManager->CreateNtupleDColumn("electronEnergy"); // MeV

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

    const G4int runID = run->GetRunID();

    G4cout
    << "\n====================================\n"
    << "Starting Run " << runID << "\n";

    if (fWriteOutput)
    {
        // intialize the analysis manager to store data for photon hit top or bottom
        auto analysisManager = G4AnalysisManager::Instance();

        const auto* detector = static_cast<const DetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction());

        std::ostringstream nameStream;
        nameStream << detector->GetScintLengthY() / mm << "mm";

        const auto* generator = static_cast<const PrimaryGeneratorAction*>(
            G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());

        // data/<material>/<energy>/<length>mm.{root,csv}
        const std::filesystem::path outputDir =
            std::filesystem::path("data")
            / std::string(detector->GetScintMaterialName())
            / EnergyLabel(generator->GetParticleGun()->GetParticleEnergy());

        std::error_code ec;
        std::filesystem::create_directories(outputDir, ec);

        if (ec)
        {
            G4cerr << "ERROR: could not create " << outputDir << ": " << ec.message() << G4endl;
        }

        const std::string basePath = (outputDir / nameStream.str()).string();

        G4cout << "Output files    : " << basePath << ".root / .csv\n";

        if (!analysisManager->OpenFile(basePath + ".root"))
        {
            G4cerr << "ERROR: Could not open ROOT file " << basePath << ".root" << G4endl;
        }

        if (!CsvHitWriter::Instance().Open(basePath + ".csv"))
        {
            G4cerr << "ERROR: Could not open CSV file " << basePath << ".csv" << G4endl;
        }
    }
    else
    {
        G4cout << "Output disabled\n";
    }

    G4cout << "====================================\n" << G4endl;

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

    // storing data in ROOT and CSV
    if (fWriteOutput)
    {
        auto analysisManager = G4AnalysisManager::Instance();
        analysisManager->Write();
        analysisManager->CloseFile();

        CsvHitWriter::Instance().Close();
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