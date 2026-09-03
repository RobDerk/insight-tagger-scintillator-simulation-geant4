#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4AnalysisManager.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4GeometryTolerance.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4Electron.hh"
#include "G4VProcess.hh"

#include <cmath>

SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) 
{

}

SteppingAction::~SteppingAction()
{

}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    if (!fScoringVolume) {
        const auto detConstruction = static_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());
        
        fScoringVolume = detConstruction->GetScoringVolume();
    }

    const auto* prePoint = step->GetPreStepPoint();

    const auto* postPoint = step->GetPostStepPoint();

    G4Track* track = step->GetTrack();

    auto* volume = prePoint->GetTouchableHandle()->GetVolume()->GetLogicalVolume();

    if (track->GetDefinition() == G4Electron::Definition()
        && track->GetParentID() == 0 // only primary electon
        && !fEventAction->IsElectronEntryTimeSet()
        && volume == fScoringVolume)
    {
        if (prePoint->GetStepStatus() == fGeomBoundary)
        {
            fEventAction->SetElectronEntryTime(prePoint->GetGlobalTime());

            G4cout
                << "Electron entered scintillator at "
                << prePoint->GetGlobalTime() / ns
                << " ns"
                << G4endl;
        }
    }

    // only optical photons inside the scintillator
    if (track->GetDefinition() != G4OpticalPhoton::Definition())
    {
        return;
    }

    // Get and store the process name, later to filter only scintillation photons
    G4String creatorName = "unknown";

    const G4VProcess* creatorProcess = track->GetCreatorProcess();
    if (creatorProcess)
    {
        creatorName = creatorProcess->GetProcessName();
    }

    // check if we are in scoring volume
    if (volume != fScoringVolume)
    {
        return;
    }

    // fScoringVolume ist bereits ein G4LogicalVolume*
    G4VSolid* solid = fScoringVolume->GetSolid();

    // Check if Solid exits
    auto* boxSolid = dynamic_cast<G4Box*>(solid);

    if (!boxSolid)
    {
        return;
    }

    // get sizes of soldi box
    G4double halfX = boxSolid->GetXHalfLength();
    G4double halfY = boxSolid->GetYHalfLength();
    G4double halfZ = boxSolid->GetZHalfLength();

    // Check if the Optical Photon reach top or bottom for count

    // Photon must come from inside the scintillator.
    const auto* preVolume = prePoint->GetPhysicalVolume();
    if (!preVolume)
    {
        return;
    }

    if (preVolume->GetLogicalVolume() != fScoringVolume)
    {
        return;
    }

    // Only steps that end on a geometry boundary.
    if (postPoint->GetStepStatus() != fGeomBoundary)
    {
        return;
    }

    const G4double y = postPoint->GetPosition().y();

    const G4double tolerance = 5.0 * G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

    const G4int trackID = track->GetTrackID();

    // time boundary hit
    const G4double globalTime = postPoint->GetGlobalTime();

    // t = 0: ElectronEntryTime
    if (!fEventAction->IsElectronEntryTimeSet())
    {
        return;
    }

    const G4double relativeTimeSinceElectronHit = globalTime - fEventAction->GetElectronEntryTime();
    
    // photon path length
    const G4double trackLength = track->GetTrackLength();

    // photonenrgy @ hit
    const G4double photonEnergy = postPoint->GetKineticEnergy();

    // Photon flight time, time since creation
    const G4double photonFlightTime = track->GetLocalTime();

    const auto* event = G4RunManager::GetRunManager()->GetCurrentEvent();
    if (!event)
    {
        return;
    }
    
    const G4int eventID = event->GetEventID();

    G4String hitType;

    if (std::abs(y - halfY) < tolerance)
    {
        // true nur beim ersten Hit dieses Photons
        if (!fEventAction->RegisterTopPhoton(trackID, globalTime))
        {
            return;
        }

        hitType = "top";
    } else if (std::abs(y + halfY) < tolerance)
    {
        if (!fEventAction->RegisterBottomPhoton(trackID, globalTime))
        {
            return;
        }

        hitType = "bottom";
    } else {
        return;
    }

    // collect energy data for storing
    auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->FillNtupleDColumn(0, eventID);
    analysisManager->FillNtupleDColumn(1, trackID);
    analysisManager->FillNtupleSColumn(2, hitType);
    analysisManager->FillNtupleDColumn(3, globalTime / ns);
    analysisManager->FillNtupleDColumn(4, relativeTimeSinceElectronHit / ns);
    analysisManager->FillNtupleDColumn(5, trackLength / mm);
    analysisManager->FillNtupleDColumn(6, photonEnergy / eV);
    analysisManager->FillNtupleDColumn(7, photonFlightTime / ns);
    analysisManager->FillNtupleSColumn(8, creatorName);

    analysisManager->AddNtupleRow();

}