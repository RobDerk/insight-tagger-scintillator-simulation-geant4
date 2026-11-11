#ifndef ActionInitialization_hh
#define ActionInitialization_hh 1

#include "G4VUserActionInitialization.hh"

#include "globals.hh"

/// Action initialization class
class ActionInitialization : public G4VUserActionInitialization
{
    public:
        // writeOutput: false in the interactive UI mode, so no CSV files are written
        explicit ActionInitialization(G4bool writeOutput = true) : fWriteOutput(writeOutput) {}
        ~ActionInitialization() override = default;

        void BuildForMaster() const override;
        void Build() const override;

    private:
        G4bool fWriteOutput = true;
};

#endif
