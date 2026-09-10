#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4Gamma.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4VSolid.hh"
#include "G4Exception.hh"

#include "Randomize.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    // Number of Particles per Event Run
    G4int num_of_particles = 1;

    fParticleGun = new G4ParticleGun(num_of_particles);

    // find particle in table and set is as gun particle
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName;
    G4ParticleDefinition* particle = particleTable->FindParticle(particleName = "e-");
    fParticleGun->SetParticleDefinition(particle);

    // Set Particles Mom Direction
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.)); // mom only in z direction

    // Set particle energy
    fParticleGun->SetParticleEnergy(1 * GeV);

    // Set Time Start of gun
    fParticleGun->SetParticleTime(0.0 * ns);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // Get world volume
    auto* logicWorld = G4LogicalVolumeStore::GetInstance()->GetVolume("World");

    auto* worldBox = dynamic_cast<G4Box*>(logicWorld->GetSolid());

    // World half length in z direction
    const G4double worldHalfZ = worldBox->GetZHalfLength();

    // Start at the negative z side of the world. The y-position is chosen
    // so the current beam direction crosses the scintillator centre.
    const G4double x0 = 0.0;
    const G4double z0 = -worldHalfZ + 1.0 * um;

    const auto direction = fParticleGun->GetParticleMomentumDirection();
    G4double y0 = 0.0;

    if (std::abs(direction.z()) > 0.0)
    {
        y0 = -direction.y() * (0.0 - z0) / direction.z();
    }

    fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));

    fParticleGun->GeneratePrimaryVertex(event);
}