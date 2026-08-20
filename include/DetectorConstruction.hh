#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh 1

#include "G4VUserDetectorConstruction.hh"

#include "G4SystemOfUnits.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;

class DetectorConstruction final : public G4VUserDetectorConstruction
{
    public:
        DetectorConstruction();
        ~DetectorConstruction() override;

        virtual G4VPhysicalVolume* Construct() override;

        virtual G4LogicalVolume* GetScoringVolume() const
        {
            return fScoringVolume;
        }
        
        void ConstructSDandField() override;

    private:
        G4LogicalVolume* fScintillatorLogical = nullptr;

    protected:
        G4LogicalVolume* fScoringVolume = nullptr;
};

#endif