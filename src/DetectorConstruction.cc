#include "DetectorConstruction.hh"
#include "ScintillatorSD.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Material.hh"
#include "G4SDManager.hh"

DetectorConstruction::DetectorConstruction()
{

}

DetectorConstruction::~DetectorConstruction()
{

}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // Nist Material Manager
    G4NistManager* nist = G4NistManager::Instance();

    // Option to switch on/off checking of volumes overlaps´
    G4bool checkOverlaps = true;

    //
    // World
    //

    // World size
    G4double world_size_x = 10 * cm;
    G4double world_size_y = 10 * cm;
    G4double world_size_z = 10 * cm;
    
    // World Material
    G4Material* world_material = nist->FindOrBuildMaterial("G4_AIR");

    auto* air_mpt = new G4MaterialPropertiesTable();
    air_mpt->AddProperty("RINDEX", "Air");
    world_material->SetMaterialPropertiesTable(air_mpt);

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

    // Scintillator Material
    G4Material* scintillator_material = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"); // Plastic Scintillator
    
    // Optical properties of the scintillator
    auto* scint_mpt = new G4MaterialPropertiesTable();

    std::vector<G4double> photonEnergy =
    {
        2.38 * eV,
        2.48 * eV,
        2.58 * eV,
        2.70 * eV,
        2.82 * eV,
        2.95 * eV,
        3.10 * eV,
        3.26 * eV,
        3.44 * eV
    };

    std::vector<G4double> refractiveIndex =
    {
        1.58,
        1.58,
        1.58,
        1.58,
        1.58,
        1.58,
        1.58,
        1.58,
        1.58
    };

    std::vector<G4double> absorptionLength =
    {
        2.0 * m,
        2.0 * m,
        2.0 * m,
        2.0 * m,
        2.0 * m,
        2.0 * m,
        2.0 * m,
        2.0 * m,
        2.0 * m
    };

    std::vector<G4double> scintSpectrum =
    {
        0.00,
        0.02,
        0.08,
        0.25,
        0.65,
        1.00,
        0.70,
        0.20,
        0.02
    };
 
    scint_mpt->AddProperty(
        "RINDEX",
        photonEnergy,
        refractiveIndex
    );

    scint_mpt->AddProperty(
        "ABSLENGTH",
        photonEnergy,
        absorptionLength
    );

    // Wavelenght of the photons
    scint_mpt->AddProperty(
        "SCINTILLATIONCOMPONENT1",
        photonEnergy,
        scintSpectrum
    );

    // Photons per deposit energy
    scint_mpt->AddConstProperty(
        "SCINTILLATIONYIELD",
        3000. / MeV
    );

    // decay time of scintillation
    scint_mpt->AddConstProperty(
        "SCINTILLATIONTIMECONSTANT1",
        2.1 * ns
    );

    // stats. fluc. of photon-yields
    scint_mpt->AddConstProperty(
        "RESOLUTIONSCALE",
        1.0
    );

    scintillator_material->SetMaterialPropertiesTable(scint_mpt);

    // Scintillator size
    G4double scint_size_x = 7 * mm;
    G4double scint_size_y = 35 * mm;
    G4double scint_size_z = 2 * mm;

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

    // Later add PMT @ top and bottom
    // //
    // // Detector: PMT
    // //

    // // Material of PMTlogicWorld
    // G4Material* pmt_material = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");

    // // sizes
    // G4double pmt_size_x = 30 * cm;
    // G4double pmt_size_y = 20 * cm;
    // G4double pmt_size_z = 5 * cm;

    // // PMT Solid Box
    // auto PMTSolid = new G4Box(
    //     "PMTSolid",
    //     pmt_size_x,
    //     pmt_size_y,
    //     pmt_size_z
    // );

    // // PMT Logical Volume
    // auto PMTLogical = new G4LogicalVolume(
    //     PMTSolid,
    //     pmt_material,
    //     "fPMTLogical"
    // );

    // G4double pmt_place_x0 = 0.0; 
    // G4double pmt_place_y0 = 0.0;
    // G4double pmt_place_z0 = 0.0;

    // auto PMTPhysical = new G4PVPlacement(
    //     nullptr,
    //     G4ThreeVector(
    //         pmt_place_x0,
    //         pmt_place_y0,
    //         pmt_place_z0
    //     ),
    //     PMTLogical,
    //     "PMTPhysical",
    //     logicWorld,
    //     false,
    //     0,
    //     checkOverlaps
    // );

    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    ScintillatorSD* scintillatorSD = new ScintillatorSD("ScintillatorSD");

    G4SDManager::GetSDMpointer()->AddNewDetector(scintillatorSD);

    SetSensitiveDetector(fScintillatorLogical, scintillatorSD);

}