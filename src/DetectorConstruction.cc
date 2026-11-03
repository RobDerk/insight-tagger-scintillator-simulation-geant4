#include "DetectorConstruction.hh"
#include "OpticalMaterials.hh"
#include "ScintillatorSD.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Material.hh"
#include "G4SDManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"

DetectorConstruction::DetectorConstruction(G4double scintLengthY, const G4String& scintLengthUnit)
    : fScintLengthY(scintLengthY), fScintLengthUnit(scintLengthUnit)
{

}

DetectorConstruction::~DetectorConstruction()
{

}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // Option to switch on/off checking of volumes overlaps´
    G4bool checkOverlaps = true;

    //
    // World
    //

    auto* world_material = OpticalMaterials::CreateAir();

    // World size
    G4double world_size_x = kWorldHalfSize;
    G4double world_size_y = kWorldHalfSize;
    G4double world_size_z = kWorldHalfSize;

    // Solid World
    auto solidWorld = new G4Box(
        "World",
        world_size_x,
        world_size_y,
        world_size_z
    );

    // Logical World - Material of the World + Physical Properties
    auto logicWorld = new G4LogicalVolume(
        solidWorld,
        world_material,
        "World"
    );

    // Physical World - Set the position of the locgical World
    auto physWorld = new G4PVPlacement(
        nullptr,
        G4ThreeVector(),
        logicWorld,
        "World",
        nullptr,
        false,
        0, 
        checkOverlaps
    );

    //
    // Detector: Scintillator
    //

    auto* scintillator_material = OpticalMaterials::CreateEJ232();
    // auto* scintillator_material = OpticalMaterials::Saphir_Glass();

    // Scintillator size
    G4double scint_size_x = 7 * mm; // length
    G4double scint_size_y = 0.5 * fScintLengthY; // G4Box takes half-lengths
    G4double scint_size_z = 2 * mm; // thickness

    // Scintillator Solid Box
    auto scintillatorSolid = new G4Box(
        "scintillatorSolid",
        scint_size_x,
        scint_size_y,
        scint_size_z
    );

    // Scintialltor Logical Vollengthume
    fScintillatorLogical =  new G4LogicalVolume(
        scintillatorSolid,
        scintillator_material,
        "fScintillatorLogical"
    );

    // Scintillator Placement
    G4double scint_place_x0 = 0.0; 
    G4double scint_place_y0 = 0.0;
    G4double scint_place_z0 = 0.0;

    // Scintillator Physical & Placement
    auto scintillatorPhysical = new G4PVPlacement(
        nullptr,
        G4ThreeVector(
            scint_place_x0,
            scint_place_y0,
            scint_place_z0
        ),
        fScintillatorLogical,
        "ScintillatorPhysical",
        logicWorld,
        false,
        0, 
        checkOverlaps
    );

    // Scintialltor Detector as ScoringVolume
    fScoringVolume = fScintillatorLogical;

    // Ideal photon collection surfaces
    const G4double collectorHalfThickness = kCollectorHalfThickness;

    // Active area of one SiPM
    const G4double activeArea_size_x = 3.72 * mm;
    const G4double activeArea_size_z = 3.62 * mm;

    // Three SiPMs per end face, evenly spread over the full end-face width
    const G4int sipmsPerFace = 3;
    const G4double sipmPitch = 2 * scint_size_x / sipmsPerFace;

    // Material is basically irrelevant because the ideal optical surface
    // absorbs/detects the photon before it propagates inside the collector
    auto* collectorSolid = new G4Box(
        "PhotonCollectorSolid",
        0.5 * activeArea_size_x,
        collectorHalfThickness,
        0.5 * activeArea_size_z
    );

    auto* collectorLogical = new G4LogicalVolume(
        collectorSolid,
        world_material,
        "PhotonCollectorLogical"
    );

    // PDE data for the AFBR-S4N44P014M NUV-MT Silicon Photomultiplier
    auto* idealDetectorSurface =
        OpticalMaterials::CreateIdealPhotonDetectorSurface(
            G4String(PDE_DATA_DIR) + "/PDE_afbr-s4n44p01M_extracted.txt"
        );

    fTopCollectors.clear();
    fBottomCollectors.clear();

    // side = +1: top end face, side = -1: bottom end face
    for (const G4int side : {1, -1})
    {
        const G4String sideName = (side > 0) ? "Top" : "Bottom";
        auto& collectors = (side > 0) ? fTopCollectors : fBottomCollectors;

        for (G4int i = 0; i < sipmsPerFace; ++i)
        {
            const G4double sipm_place_x0 = scint_place_x0 + (i - 0.5 * (sipmsPerFace - 1)) * sipmPitch;
            const G4double sipm_place_y0 = scint_place_y0 + side * (scint_size_y + collectorHalfThickness);
            const G4double sipm_place_z0 = scint_place_z0;

            const G4String sipmName = sideName + "SiPM" + std::to_string(i);

            auto* sipmPhysical = new G4PVPlacement(
                nullptr,
                G4ThreeVector(
                    sipm_place_x0,
                    sipm_place_y0,
                    sipm_place_z0
                ),
                collectorLogical,
                sipmName,
                logicWorld,
                false,
                i,
                checkOverlaps
            );

            new G4LogicalBorderSurface(
                "ScintillatorTo" + sipmName + "Surface",
                scintillatorPhysical,
                sipmPhysical,
                idealDetectorSurface
            );

            collectors.push_back(sipmPhysical);
        }
    }

    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    ScintillatorSD* scintillatorSD = new ScintillatorSD("ScintillatorSD");

    G4SDManager::GetSDMpointer()->AddNewDetector(scintillatorSD);

    SetSensitiveDetector(fScintillatorLogical, scintillatorSD);
}