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
        static constexpr G4double kWorldHalfSize = 10 * cm;
        static constexpr G4double kCollectorHalfThickness = 0.5 * um;

        // Largest full length for which both end-face collectors still fit in the world
        static constexpr G4double kMaxScintLengthY =
            2 * (kWorldHalfSize - 2 * kCollectorHalfThickness);

        // scintLengthY: full length; halved internally for G4Box
        explicit DetectorConstruction(
            G4double scintLengthY = 70 * mm,
            const G4String& scintLengthUnit = "mm");
        ~DetectorConstruction() override;

        virtual G4VPhysicalVolume* Construct() override;

        virtual G4LogicalVolume* GetScoringVolume() const
        {
            return fScoringVolume;
        }
        
        void ConstructSDandField() override;

        // Takes effect after /run/reinitialize
        void SetScintLengthY(G4double length, const G4String& unit)
        {
            fScintLengthY = length;
            fScintLengthUnit = unit;
        }

        G4double GetScintLengthY() const
        {
            return fScintLengthY;
        }

        // Unit the length was given in, for display only
        const G4String& GetScintLengthUnit() const
        {
            return fScintLengthUnit;
        }

        G4VPhysicalVolume* GetTopCollectorPhysical() const
        {
            return fTopCollectorPhysical;
        }

        G4VPhysicalVolume* GetBottomCollectorPhysical() const
        {
            return fBottomCollectorPhysical;
        }

    private:
        // Full length of the scintillator along y (photon transport direction)
        G4double fScintLengthY = 70 * mm;
        G4String fScintLengthUnit = "mm";

        G4LogicalVolume* fScintillatorLogical = nullptr;

        G4VPhysicalVolume* fTopCollectorPhysical = nullptr;
        G4VPhysicalVolume* fBottomCollectorPhysical = nullptr;

    protected:
        G4LogicalVolume* fScoringVolume = nullptr;
};

#endif