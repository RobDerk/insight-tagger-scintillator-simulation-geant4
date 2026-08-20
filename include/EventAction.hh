#ifndef EventAction_hh 
#define EventAction_hh 1

#include "G4UserEventAction.hh"

#include "globals.hh"

#include <unordered_set>

class RunAction;
class G4Event;

class EventAction final : public G4UserEventAction
{
    public:
        EventAction(RunAction* runAction);
        ~EventAction() override;

        void BeginOfEventAction(const G4Event* event) override;
        void EndOfEventAction(const G4Event* event) override;

        // Check if Photon reach/hit top or bottom: true, false
        G4bool RegisterTopPhoton(G4int trackID, G4double time);
        G4bool RegisterBottomPhoton(G4int trackID, G4double time);

        // Energy Depostit Sum
        void AddEdep(G4double edep)
        {
            fEdep += edep; 
        }

    private:
        RunAction* fRunAction = nullptr;

        // Energy deposit
        G4double fEdep = 0.;

        // Hit counts of Photon that reach/hit Top oder Bottom of the Scintillator
        G4int fNTop = 0;
        G4int fNBottom = 0;

        // photon is counted only once in total: whichever end face it reaches first.
        std::unordered_set<G4int> fCountedPhotons;
};

#endif