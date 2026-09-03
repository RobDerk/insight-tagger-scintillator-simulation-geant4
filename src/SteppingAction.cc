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
    if (!fScoringVolume)
    {
        const auto detConstruction = static_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());
        
        fScoringVolume = detConstruction->GetScoringVolume();

        auto* boxSolid = dynamic_cast<G4Box*>(fScoringVolume->GetSolid());

        if (boxSolid)
        {
            fScintHalfX = boxSolid->GetXHalfLength();
            fScintHalfY = boxSolid->GetYHalfLength();
            fScintHalfZ = boxSolid->GetZHalfLength();
        }
    }

    const auto* prePoint = step->GetPreStepPoint();
    const auto* postPoint = step->GetPostStepPoint();

    const auto* preVolume = prePoint->GetPhysicalVolume();
    const auto* postVolume = postPoint->GetPhysicalVolume();

    G4Track* track = step->GetTrack();

    // Logical volume of current step
    const G4LogicalVolume* volume = nullptr;

    if (preVolume)
    {
        volume = preVolume->GetLogicalVolume();
    }

    if (track->GetDefinition() == G4Electron::Definition()
        && track->GetParentID() == 0 // only primary electon
        && !fEventAction->IsElectronEntryTimeSet())
    {
        // Step crosses a geometry boundary
        if (postPoint->GetStepStatus() == fGeomBoundary
            && postVolume != nullptr
            && postVolume->GetLogicalVolume() == fScoringVolume
        )
        {
            // Make sure it really came from outside
            if (preVolume == nullptr || preVolume->GetLogicalVolume() != fScoringVolume)
            {
                const G4double entryTime = postPoint->GetGlobalTime();
    
                fEventAction->SetElectronEntryTime(entryTime);
            }
        }
    }

    // only optical photons inside the scintillator
    if (track->GetDefinition() != G4OpticalPhoton::Definition())
    {
        return;
    }

    // check if we are in scoring volume
    // Photon must come from inside the scintillator.
    if (volume != fScoringVolume)
    {
        return;
    }

    // get sizes of soldi box
    const G4double halfX = fScintHalfX;
    const G4double halfY = fScintHalfY;
    const G4double halfZ = fScintHalfZ;

    // Check if the Optical Photon reach top or bottom for count

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

    // Get and store the process name, later to filter only scintillation photons
    const auto* creatorProcess = track->GetCreatorProcess();

    G4String creatorName = "unknown";
    if (creatorProcess)
    {
        creatorName = creatorProcess->GetProcessName();
    }

    // Get runID
    const G4int runID = G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID();

    // Calculcate the photon creation time
    const G4double photonCreationTime = relativeTimeSinceElectronHit - photonFlightTime;

    // collect energy data for storing
    auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(0, eventID);
    analysisManager->FillNtupleIColumn(1, eventID);
    analysisManager->FillNtupleIColumn(2, trackID);
    analysisManager->FillNtupleSColumn(3, hitType);
    analysisManager->FillNtupleDColumn(4, globalTime / ns);
    analysisManager->FillNtupleDColumn(5, relativeTimeSinceElectronHit / ns);
    analysisManager->FillNtupleDColumn(6, trackLength / mm);
    analysisManager->FillNtupleDColumn(7, photonEnergy / eV);
    analysisManager->FillNtupleDColumn(8, photonFlightTime / ns);
    analysisManager->FillNtupleDColumn(8, photonCreationTime / ns);
    analysisManager->FillNtupleSColumn(9, creatorName);

    analysisManager->AddNtupleRow();

}