#ifndef ScintillatorSD_hh
#define ScintillatorSD_hh 1

#include "G4VSensitiveDetector.hh"

class G4Step;
class G4TouchableHistory;

class ScintillatorSD final : public G4VSensitiveDetector
{
    public:
        ScintillatorSD(G4String name);
        ~ScintillatorSD() override;

        G4bool ProcessHits(
            G4Step* step,
            G4TouchableHistory* history
        ) override;
};

#endif