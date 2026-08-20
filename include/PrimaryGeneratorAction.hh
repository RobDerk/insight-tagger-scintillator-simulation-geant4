#ifndef PrimaryGeneratorAction_hh
#define PrimaryGeneratorAction_hh 1

#include "G4VUserPrimaryGeneratorAction.hh"

class G4Event;
class G4ParticleGun;
class G4Box;

// primary generator action with particle gun
class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
    private:
        G4ParticleGun* fParticleGun = nullptr;
        G4Box* fWorldBox = nullptr;

    public:
        PrimaryGeneratorAction();
        ~PrimaryGeneratorAction() override;

        void GeneratePrimaries(G4Event* event) override;

        // method to access particle gun
        const G4ParticleGun* GetParticleGun() const { 
            return fParticleGun;
        };
};

#endif