#ifndef EventAction_hh 
#define EventAction_hh 1

#include "G4UserEventAction.hh"

#include "globals.hh"

#include <chrono>
#include <unordered_map>
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
        G4bool RegisterTopPhoton(
            G4int trackID,
            G4double time
        );
        G4bool RegisterBottomPhoton(
            G4int trackID,
            G4double time
        );

        // Energy Depostit Sum
        void AddEdep(G4double edep)
        {
            fEdep += edep; 
        }

        // Set electron entry time
        // Set time start @ electron hit
        void SetElectronEntryTime(G4double time);

        G4double GetElectronEntryTime() const
        {
            return fElectronEntryTime;
        }
    
        G4bool IsElectronEntryTimeSet() const
        {
            return fElectronTimeSet;
        }

        // Reflections of a photon at the scintillator side faces
        void AddSideReflection(G4int trackID)
        {
            ++fSideReflections[trackID];
        }

        G4int GetSideReflections(G4int trackID) const
        {
            const auto it = fSideReflections.find(trackID);
            return it == fSideReflections.end() ? 0 : it->second;
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

        // trackID -> number of side-face reflections
        std::unordered_map<G4int, G4int> fSideReflections;

        // Set the start time when the electron hit the scintillator
        G4double fElectronEntryTime = -1.0;
        G4bool fElectronTimeSet = false; // check if flag is set

        // Wall-clock start of the event
        std::chrono::steady_clock::time_point fEventStart;
};

#endif