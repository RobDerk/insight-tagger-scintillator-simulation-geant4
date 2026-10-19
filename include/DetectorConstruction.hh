#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh 1

#include "G4VUserDetectorConstruction.hh"
#include "OpticalMaterials.hh"
#include "G4SystemOfUnits.hh"

#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;

class DetectorConstruction final : public G4VUserDetectorConstruction
{
    public:
        explicit DetectorConstruction(G4double scintHalfLengthY = 35 * mm);
        ~DetectorConstruction() override;

        virtual G4VPhysicalVolume* Construct() override;

        virtual G4LogicalVolume* GetScoringVolume() const
        {
            return fScoringVolume;
        }
        
        void ConstructSDandField() override;

        G4VPhysicalVolume* GetTopCollectorPhysical() const
        {
            return fTopCollectorPhysical;
        }

        G4VPhysicalVolume* GetBottomCollectorPhysical() const
        {
            return fBottomCollectorPhysical;
        }

    private:
        // Half-length of the scintillator along y (photon transport direction)
        G4double fScintHalfLengthY = 35 * mm;

        G4LogicalVolume* fScintillatorLogical = nullptr;

        G4VPhysicalVolume* fTopCollectorPhysical = nullptr;
        G4VPhysicalVolume* fBottomCollectorPhysical = nullptr;

    protected:
        G4LogicalVolume* fScoringVolume = nullptr;
};

#endif