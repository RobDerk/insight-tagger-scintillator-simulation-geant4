#include "ScintillatorSD.hh"

#include "G4Step.hh"
#include "G4SystemOfUnits.hh"

ScintillatorSD::ScintillatorSD(G4String name) : G4VSensitiveDetector(name)
{

}

ScintillatorSD::~ScintillatorSD()
{

}

G4bool ScintillatorSD::ProcessHits(G4Step* step, G4TouchableHistory* history)
{
    // // Energy deposit in the step
    // G4double edep_step = step->GetTotalEnergyDeposit();

    // if (edep_step <= 0.) 
    // {
    //     return false;
    // }

    // // Calc Pos.
    // G4StepPoint* thePrePoint = step->GetPreStepPoint();
    // G4StepPoint* thePostPoint = step->GetPostStepPoint();

    // // the average pos of the hit
    // G4ThreeVector pos = ((thePrePoint->GetPosition() + thePostPoint->GetPosition()) / 2);

    // G4cout << "Edep step = "
    //         << edep_step / keV
    //         << " keV"
    //         << " - Position = "
    //         << pos
    //         << G4endl;

    // G4VPhysicalVolume* preVol = step->GetPreStepPoint()->GetPhysicalVolume();
    // G4Material* postVol = step->GetPostStepPoint()->GetMaterial();
    
    // G4cout << "Volume: "
    //         << postVol
    //         << G4endl;

    return true;
}