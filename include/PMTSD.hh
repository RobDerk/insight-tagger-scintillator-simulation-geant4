#ifndef PMTSD_hh
#define PMTSD_hh 1

#include "G4VSensitiveDetector.hh"

class G4Step;
class G4TouchableHistory;

class PMTSD final : public G4VSensitiveDetector
{
    public:
        PMTSD(G4String name);
        ~PMTSD() override;

        G4bool ProcessHits(
            G4Step* step,
            G4TouchableHistory* history
        ) override;
};

#endif