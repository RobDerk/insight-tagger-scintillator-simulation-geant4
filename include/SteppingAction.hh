#ifndef B1SteppingAction_h
#define B1SteppingAction_h 1

#include "G4UserSteppingAction.hh"

#include "globals.hh"

#include <vector>

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Step;
class EventAction;

/// Stepping action class
class SteppingAction : public G4UserSteppingAction
{
  public:
    SteppingAction(EventAction* eventAction, G4bool writeOutput = true);
    ~SteppingAction() override;

    // method from the base class
    void UserSteppingAction(const G4Step*) override;

  private:
    EventAction* fEventAction = nullptr;
    G4bool fWriteOutput = true;
    G4LogicalVolume* fScoringVolume = nullptr;
    G4int fCachedRunID = -1;

    std::vector<G4VPhysicalVolume*> fTopCollectors;
    std::vector<G4VPhysicalVolume*> fBottomCollectors;

    G4double fScintHalfX = 0.0;
    G4double fScintHalfY = 0.0;
    G4double fScintHalfZ = 0.0;
};

#endif
