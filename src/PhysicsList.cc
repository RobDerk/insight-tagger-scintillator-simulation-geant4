#include "PhysicsList.hh"

#include "G4EmStandardPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4DecayPhysics.hh"

PhysicsList::PhysicsList()
{
    SetVerboseLevel(1);

    // Standard elektromagnetische Prozesse:
    // Ionisation, Bremsstrahlung, Compton, Photoeffekt, ..
    RegisterPhysics(new G4EmStandardPhysics);

    // Alle Standard Optical-Physics-Prozesse
    RegisterPhysics(new G4OpticalPhysics);

    // Normale Zerfälle
    RegisterPhysics(new G4DecayPhysics());
}

PhysicsList::~PhysicsList()
{

}