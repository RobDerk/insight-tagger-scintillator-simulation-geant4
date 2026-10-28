#include "EventAction.hh"
#include "RunAction.hh"
#include "DetectorConstruction.hh"

#include "G4Event.hh"
#include "G4ParticleDefinition.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4RunManager.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include <iomanip>

EventAction::EventAction(RunAction* runAction) : fRunAction(runAction)
{

}

EventAction::~EventAction()
{
    
}

// At the Beginn of the Action, E is zero
void EventAction::BeginOfEventAction(const G4Event*)
{
    fEventStart = std::chrono::steady_clock::now();

    fElectronEntryTime = 0.0;
    fElectronTimeSet = false;

    // Set energy and counts to zero
    fEdep = 0.;

    fNTop = 0;
    fNBottom = 0;

    fCountedPhotons.clear();
    fSideReflections.clear();
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    const auto eventID = event->GetEventID();

    fRunAction->WritePhotonCounts(eventID, fNTop, fNBottom);

    const G4double wallTime =
        std::chrono::duration<G4double>(
            std::chrono::steady_clock::now() - fEventStart
        ).count();

    const G4int nTotal = fNTop + fNBottom;

    // Primary particle info
    G4String particleName = "n/a";
    G4double primaryEnergy = 0.;
    G4double fScintLengthY = 0.;

    if (const auto* detector = static_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction()))
    {
        fScintLengthY = detector->GetScintLengthY();
    }

    if (const auto* vertex = event->GetPrimaryVertex())
    {
        if (const auto* primary = vertex->GetPrimary())
        {
            particleName = primary->GetParticleDefinition()->GetParticleName();
            primaryEnergy = primary->GetKineticEnergy();
        }
    }

    const G4double topFraction = nTotal > 0 ? 100. * fNTop / nTotal : 0.;
    const G4double bottomFraction = nTotal > 0 ? 100. * fNBottom / nTotal : 0.;

    G4cout
    << "\n------------------------------------\n"
    << "Event " << eventID << "\n"
    << "  Primary Particle  : " << particleName << "\n"
    << "  Primary Energy    : " << G4BestUnit(primaryEnergy, "Energy") << "\n"
    << "  Length Y          : " << G4BestUnit(fScintLengthY, "Length") << "\n"
    << "  Energy deposit    : " << G4BestUnit(fEdep, "Energy") << "\n"
    << "  Electron entry t  : ";

    if (fElectronTimeSet)
    {
        G4cout << G4BestUnit(fElectronEntryTime, "Time");
    }
    else
    {
        G4cout << "not set";
    }

    G4cout
    << "\n"
    << "  Photons N_top     : " << fNTop
    << " (" << std::fixed << std::setprecision(1) << topFraction << " %)\n"
    << "  Photons N_bottom  : " << fNBottom
    << " (" << bottomFraction << " %)\n"
    << "  Photons N_total   : " << nTotal << "\n"
    << "  Simulation time   : " << std::setprecision(3) << wallTime << " s\n"
    << "------------------------------------"
    << std::defaultfloat
    << G4endl;

    // energy deposit of electon; accumulate the stats in run action
    fRunAction->AddEdep(fEdep);
}

G4bool EventAction::RegisterTopPhoton(G4int trackID, G4double time)
{
    if (!fElectronTimeSet)
    {
        return false;
    }

    const auto result = fCountedPhotons.insert(trackID);

    if (!result.second)
    {
        return false;
    }

    if (!fElectronTimeSet)
    {
        return false;
    }

    G4double relativeTime = time - fElectronEntryTime;

    ++fNTop;

    return true;
}

G4bool EventAction::RegisterBottomPhoton(G4int trackID, G4double time)
{
    if (!fElectronTimeSet)
    {
        return false;
    }

    const auto result = fCountedPhotons.insert(trackID);

    if (!result.second)
    {
        return false;
    }

    if (!fElectronTimeSet)
    {
        return false;
    }

    G4double relativeTime = time - fElectronEntryTime;

    ++fNBottom;

    return true;
}

void EventAction::SetElectronEntryTime(G4double time)
{
    if (fElectronTimeSet) 
        return;

    fElectronEntryTime = time;
    fElectronTimeSet = true;
}