#include "SteppingAction.hh"

#include "CsvHitWriter.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4Run.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4AnalysisManager.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4GeometryTolerance.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4Electron.hh"
#include "G4VProcess.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"

#include <algorithm>

#include <cmath>

SteppingAction::SteppingAction(EventAction* eventAction, G4bool writeOutput)
    : fEventAction(eventAction), fWriteOutput(writeOutput)
{

}

SteppingAction::~SteppingAction()
{

}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Geometry is rebuilt between runs, so the cached pointers must be refreshed
    const G4int currentRunID = G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID();

    if (!fScoringVolume || currentRunID != fCachedRunID)
    {
        fCachedRunID = currentRunID;

        const auto detConstruction = static_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());
        
        fScoringVolume = detConstruction->GetScoringVolume();

        fTopCollectors = detConstruction->GetTopCollectors();
        fBottomCollectors = detConstruction->GetBottomCollectors();

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

    // Sum energy deposit of all particles in the scintillator
    if (volume == fScoringVolume)
    {
        fEventAction->AddEdep(step->GetTotalEnergyDeposit());
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

    // track only photons that were created inside the scintillator
    const auto* birthVolume = track->GetLogicalVolumeAtVertex();

    if (birthVolume != fScoringVolume)
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

    // Determine status of optical boundary process
    G4OpBoundaryProcessStatus boundaryStatus = Undefined;

    auto* processManager = G4OpticalPhoton::OpticalPhotonDefinition()->GetProcessManager();

    auto* processVector = processManager->GetPostStepProcessVector(typeDoIt);

    const G4int numberOfProcesses = processVector->entries();

    for (G4int i = 0; i < numberOfProcesses; ++i)
    {
        auto* process = (*processVector)[i];

        auto* boundaryProcess = dynamic_cast<G4OpBoundaryProcess*>(process);

        if (boundaryProcess)
        {
            boundaryStatus = boundaryProcess->GetStatus();
            break;
        }
    }

    // Count reflections at the side faces (not at the SiPM faces)
    const auto isIn = [postVolume](const std::vector<G4VPhysicalVolume*>& volumes)
    {
        return std::find(volumes.begin(), volumes.end(), postVolume) != volumes.end();
    };

    const bool hitsTop = isIn(fTopCollectors);
    const bool hitsBottom = isIn(fBottomCollectors);

    if (!hitsTop
        && !hitsBottom
        && (boundaryStatus == FresnelReflection
            || boundaryStatus == TotalInternalReflection
            || boundaryStatus == LambertianReflection
            || boundaryStatus == LobeReflection
            || boundaryStatus == SpikeReflection
            || boundaryStatus == BackScattering))
    {
        fEventAction->AddSideReflection(track->GetTrackID());
    }

    // Get the position of the step point
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

    if (boundaryStatus != Detection)
    {
        return;
    }

    G4String hitType;

    if (hitsTop)
    {
        hitType = "top";
    }

    else if (hitsBottom)
    {
        hitType = "bottom";
    }

    else
    {
        return;
    }

    // Detection is terminal
    track->SetTrackStatus(fStopAndKill);

    // Get and store the process name
    const auto* creatorProcess = track->GetCreatorProcess();

    G4String creatorName = "unknown";

    if (creatorProcess)
    {
        creatorName = creatorProcess->GetProcessName();
    }


    // Prevent double counting

    if (hitType == "top")
    {
        if (!fEventAction->RegisterTopPhoton(trackID, globalTime))
        {
            return;
        }
    }
    else
    {
        if (!fEventAction->RegisterBottomPhoton(trackID, globalTime))
        {
            return;
        }
    }

    // Emission angle between photon creation direction and primary beam direction
    G4double emissionAngle = 0.0;
    G4double electronEnergy = 0.0;

    if (const auto* vertex = event->GetPrimaryVertex(0))
    {
        if (const auto* primary = vertex->GetPrimary(0))
        {
            const G4double cosTheta = track->GetVertexMomentumDirection().dot(primary->GetMomentumDirection().unit());

            emissionAngle = std::acos(std::max(-1.0, std::min(1.0, cosTheta)));
            electronEnergy = primary->GetKineticEnergy();
        }
    }

    // Get runID
    const G4int runID = G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID();

    // Calculcate the photon creation time
    const G4double photonCreationTime = relativeTimeSinceElectronHit - photonFlightTime;

    // Without output the hit is still counted above, but no row is written
    if (!fWriteOutput)
    {
        return;
    }

    // collect energy data for storing
    HitRecord record;
    record.runID = runID;
    record.eventID = eventID;
    record.trackID = trackID;
    record.hit = hitType;
    record.globalTime = globalTime / ns;
    record.timeSinceElectronHit = relativeTimeSinceElectronHit / ns;
    record.trackLength = trackLength / mm;
    record.photonEnergy = photonEnergy / eV;
    record.photonFlightTime = photonFlightTime / ns;
    record.photonCreationTime = photonCreationTime / ns;
    record.creatorProcess = creatorName;
    record.nSideReflections = fEventAction->GetSideReflections(trackID);
    record.scintLength = 2. * fScintHalfY / mm;
    record.emissionAngle = emissionAngle / deg;
    record.material = fScoringVolume->GetMaterial()->GetName();
    record.electronEnergy = electronEnergy / MeV;

    // ROOT ntuple
    auto analysisManager = G4AnalysisManager::Instance();

    analysisManager->FillNtupleIColumn(0, record.runID);
    analysisManager->FillNtupleIColumn(1, record.eventID);
    analysisManager->FillNtupleIColumn(2, record.trackID);
    analysisManager->FillNtupleSColumn(3, record.hit);
    analysisManager->FillNtupleDColumn(4, record.globalTime);
    analysisManager->FillNtupleDColumn(5, record.timeSinceElectronHit);
    analysisManager->FillNtupleDColumn(6, record.trackLength);
    analysisManager->FillNtupleDColumn(7, record.photonEnergy);
    analysisManager->FillNtupleDColumn(8, record.photonFlightTime);
    analysisManager->FillNtupleDColumn(9, record.photonCreationTime);
    analysisManager->FillNtupleSColumn(10, record.creatorProcess);
    analysisManager->FillNtupleIColumn(11, record.nSideReflections);
    analysisManager->FillNtupleDColumn(12, record.scintLength);
    analysisManager->FillNtupleDColumn(13, record.emissionAngle);
    analysisManager->FillNtupleSColumn(14, record.material);
    analysisManager->FillNtupleDColumn(15, record.electronEnergy);

    analysisManager->AddNtupleRow();

    // CSV file
    CsvHitWriter::Instance().Write(record);
}