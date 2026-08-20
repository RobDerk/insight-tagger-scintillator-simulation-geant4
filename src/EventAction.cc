#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4ios.hh"

EventAction::EventAction(RunAction* runAction) : fRunAction(runAction)
{

}

EventAction::~EventAction()
{
    
}

// At the Beginn of the Action, E is zero
void EventAction::BeginOfEventAction(const G4Event*)
{
    // Set energy and counts to zero
    fEdep = 0.;
    fNTop = 0;
    fNBottom = 0;
    fCountedPhotons.clear();
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    const auto eventID = event->GetEventID();

    fRunAction->WritePhotonCounts(eventID, fNTop, fNBottom);

    if (eventID < 10)
    {
        G4cout
            << "Event " << eventID
            << ": N_top=" << fNTop
            << ", N_bottom=" << fNBottom
            << ", N_total=" << (fNTop + fNBottom)
            << G4endl;
    }

    // energy deposit of electon; accumulate the stats in run action
    fRunAction->AddEdep(fEdep);
}

G4bool EventAction::RegisterTopPhoton(G4int trackID, G4double time)
{
    const auto result = fCountedPhotons.insert(trackID);
    if (!result.second)
    {
        return false;
    }

    G4double timeInNs = time;

    ++fNTop;
    return true;
}

G4bool EventAction::RegisterBottomPhoton(G4int trackID, G4double time)
{
    const auto result = fCountedPhotons.insert(trackID);
    if (!result.second)
    {
        return false;
    }

    G4double timeInNs = time;

    ++fNBottom;
    return true;
}