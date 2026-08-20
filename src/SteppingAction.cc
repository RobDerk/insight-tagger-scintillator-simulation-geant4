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
#include "G4Box.hh"

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

    // get volume of the current step
    G4LogicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();

    // check if we are in scoring volume
    if (volume != fScoringVolume)
    {
        return;
    }

    // fScoringVolume ist bereits ein G4LogicalVolume*
    G4VSolid* solid = fScoringVolume->GetSolid();

    G4double halfX = 0.0;
    G4double halfY = 0.0;
    G4double halfZ = 0.0;

    // Typumwandlung zu G4Box
    if (G4Box* boxSolid = dynamic_cast<G4Box*>(solid)) {
        halfX = boxSolid->GetXHalfLength();
        halfY = boxSolid->GetYHalfLength();
        halfZ = boxSolid->GetZHalfLength();
    }

    G4Track* track = step->GetTrack();

    // Check if the Optical Photon reach top or bottom for count
    
    // Only optical photons.
    if (track->GetDefinition() != G4OpticalPhoton::Definition())
    {
        return;
    }

    const auto* prePoint = step->GetPreStepPoint();
    const auto* postPoint = step->GetPostStepPoint();


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

    const auto& position = postPoint->GetPosition();
    const G4double y = position.y();

    const G4double tolerance = 5.0 * G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();

    const G4int trackID = track->GetTrackID();
    G4double localTime = track->GetLocalTime();
    G4double globalTime = track->GetGlobalTime(); 

    // collect energy data for storing
    auto analysisManager = G4AnalysisManager::Instance();

    // Top face: y = +halfY
    if (std::abs(y - halfY) < tolerance)
    {
        fEventAction->RegisterTopPhoton(trackID, globalTime);

        analysisManager->FillNtupleSColumn(1, "top");

        G4cout << "Hit top"
                << " Global Time: "
                << globalTime
                << " Local Time: "
                << localTime
                << G4endl;

        return;
    }

    // Bottom face: y = -halfY
    if (std::abs(y + halfY) < tolerance)
    {
        fEventAction->RegisterBottomPhoton(trackID, globalTime);

        analysisManager->FillNtupleSColumn(1, "bottom");

        G4cout << "Hit bottom"
                << " Global Time: "
                << globalTime
                << " Local Time: "
                << localTime
                << G4endl;

        return;
    }

    analysisManager->FillNtupleDColumn(2, globalTime);
    analysisManager->FillNtupleDColumn(3, localTime);

    analysisManager->AddNtupleRow();

    // G4Track* track = step->GetTrack();

    // // collect information about the step for data
    // G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    // G4double edepStep = step->GetTotalEnergyDeposit();
    // G4double kineticEnergy = track->GetKineticEnergy();
    // G4ThreeVector position = step->GetDeltaPosition();
    // G4ThreeVector momentum = track->GetMomentum();
    // G4double edepStepLen = step->GetStepLength();
    // G4ThreeVector edepStepPos = step->GetPostStepPoint()->GetPosition();
    // G4double tofStep = step->GetDeltaTime();
    // G4double globalTime = track->GetGlobalTime();
    // G4int trackID = track->GetTrackID();
    // G4int parentID = track->GetParentID();
    // G4String particleName = track->GetParticleDefinition()->GetParticleName();

    // // add the energy dep
    // fEventAction->AddEdep(edepStep);

    // // collect energy data for storing
    // auto analysisManager = G4AnalysisManager::Instance();


    // // Optinal if parentID == 0 -> this is a primary track; save only primary tracks

    // if (edepStep > 0.)
    // {
    //     analysisManager->FillNtupleDColumn(0, eventID);
    //     analysisManager->FillNtupleDColumn(1, edepStep);
    //     analysisManager->FillNtupleDColumn(2, kineticEnergy);
    //     analysisManager->FillNtupleDColumn(3, position.x());
    //     analysisManager->FillNtupleDColumn(4, position.y());
    //     analysisManager->FillNtupleDColumn(5, position.z());
    //     analysisManager->FillNtupleDColumn(6, momentum.z());
    //     analysisManager->FillNtupleDColumn(7, momentum.z());
    //     analysisManager->FillNtupleDColumn(8, momentum.z());
    //     analysisManager->FillNtupleDColumn(9, edepStepLen);
    //     analysisManager->FillNtupleDColumn(10, edepStepPos.x());
    //     analysisManager->FillNtupleDColumn(11, edepStepPos.y());
    //     analysisManager->FillNtupleDColumn(12, edepStepPos.z());
    //     analysisManager->FillNtupleDColumn(13, tofStep);
    //     analysisManager->FillNtupleDColumn(14, globalTime);
    //     analysisManager->FillNtupleDColumn(15, trackID);
    //     analysisManager->FillNtupleDColumn(16, parentID);
    //     analysisManager->FillNtupleSColumn(17, particleName);

    //     analysisManager->AddNtupleRow();
    // }

}