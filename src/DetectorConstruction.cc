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

DetectorConstruction::DetectorConstruction()
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
    G4double world_size_x = 10 * cm;
    G4double world_size_y = 10 * cm;
    G4double world_size_z = 10 * cm;

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

    // Scintillator size
    G4double scint_size_x = 7 * mm; // length
    G4double scint_size_y = 35 * mm; // width
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

    const G4double collectorHalfThickness = 0.5 * um;

    // Same x/z size as scintillator end face
    auto* collectorSolid = new G4Box(
        "PhotonCollectorSolid",
        scint_size_x,
        collectorHalfThickness,
        scint_size_z
    );

    // Material is basically irrelevant because the ideal optical surface
    // absorbs/detects the photon before it propagates inside the collector
    auto* collectorLogical = new G4LogicalVolume(
        collectorSolid,
        world_material,
        "PhotonCollectorLogical"
    );

    // Top collector
    fTopCollectorPhysical = new G4PVPlacement(
        nullptr,
        G4ThreeVector(
            scint_place_x0,
            scint_place_y0 + scint_size_y + collectorHalfThickness,
            scint_place_z0
        ),
        collectorLogical,
        "TopPhotonCollector",
        logicWorld,
        false,
        0,
        checkOverlaps
    );

    // Bottom collector
    fBottomCollectorPhysical = new G4PVPlacement(
        nullptr,
        G4ThreeVector(
            scint_place_x0,
            scint_place_y0 - scint_size_y - collectorHalfThickness,
            scint_place_z0
        ),
        collectorLogical,
        "BottomPhotonCollector",
        logicWorld,
        false,
        1,
        checkOverlaps
    );

    auto* idealDetectorSurface =
        OpticalMaterials::CreateIdealPhotonDetectorSurface();

    new G4LogicalBorderSurface(
        "ScintillatorToTopCollectorSurface",
        scintillatorPhysical,
        fTopCollectorPhysical,
        idealDetectorSurface
    );

    new G4LogicalBorderSurface(
        "ScintillatorToBottomCollectorSurface",
        scintillatorPhysical,
        fBottomCollectorPhysical,
        idealDetectorSurface
    );

    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    ScintillatorSD* scintillatorSD = new ScintillatorSD("ScintillatorSD");

    G4SDManager::GetSDMpointer()->AddNewDetector(scintillatorSD);

    SetSensitiveDetector(fScintillatorLogical, scintillatorSD);
}