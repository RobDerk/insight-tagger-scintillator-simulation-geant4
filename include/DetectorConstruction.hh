#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh 1

#include "G4VUserDetectorConstruction.hh"
#include "OpticalMaterials.hh"
#include "G4SystemOfUnits.hh"

#include "globals.hh"

#include <vector>

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
            const G4String& scintLengthUnit = "mm",
            const G4String& scintMaterial = "EJ232");
        ~DetectorConstruction() override;

        virtual G4VPhysicalVolume* Construct() override;

        virtual G4LogicalVolume* GetScoringVolume() const
        {
            return fScoringVolume;
        }
        
        void ConstructSDandField() override;

        // Takes effect after /run/reinitializeGeometry
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

        const G4String& GetScintMaterialName() const
        {
            return fScintMaterialName;
        }

        // SiPM active areas on the top (+y) and bottom (-y) end face
        const std::vector<G4VPhysicalVolume*>& GetTopCollectors() const
        {
            return fTopCollectors;
        }

        const std::vector<G4VPhysicalVolume*>& GetBottomCollectors() const
        {
            return fBottomCollectors;
        }

    private:
        // Full length of the scintillator along y (photon transport direction)
        G4double fScintLengthY = 70 * mm;
        G4String fScintLengthUnit = "mm";
        G4String fScintMaterialName = "EJ232";

        G4LogicalVolume* fScintillatorLogical = nullptr;

        std::vector<G4VPhysicalVolume*> fTopCollectors;
        std::vector<G4VPhysicalVolume*> fBottomCollectors;

    protected:
        G4LogicalVolume* fScoringVolume = nullptr;
};

#endif