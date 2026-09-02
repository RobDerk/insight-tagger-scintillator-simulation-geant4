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
#include "G4PhysicalConstants.hh"

#include <vector>

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

    auto* world_material = CreateAir();

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

    auto* scintillator_material = CreateEJ232();

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

G4Material* DetectorConstruction::CreateAir()
{
    auto* nist = G4NistManager::Instance();

    // air material
    G4Material* world_material = nist->FindOrBuildMaterial("G4_AIR");

    std::vector<G4double> photonEnergy =
    {
        2.696 * eV,
        2.818 * eV,
        2.952 * eV,
        3.100 * eV,
        3.179 * eV,
        3.263 * eV,
        3.351 * eV,
        3.444 * eV,
        3.542 * eV,
        3.647 * eV
    };


    std::vector<G4double> airRIndex =
    {
        1.0003,
        1.0003,
        1.0003,
        1.0003,
        1.0003,
        1.0003,
        1.0003,
        1.0003,
        1.0003,
        1.0003
    };

    auto* air_mpt = new G4MaterialPropertiesTable();

    air_mpt->AddProperty(
        "RINDEX",
        photonEnergy,
        airRIndex
    );

    world_material->SetMaterialPropertiesTable(air_mpt);

    return world_material;
}

G4Material* DetectorConstruction::CreateEJ232()
{
    auto* nist = G4NistManager::Instance();

    auto* H = nist->FindOrBuildElement("H");
    auto* C = nist->FindOrBuildElement("C");

    // material
    auto* EJ232 =
        new G4Material(
            "EJ232",
            1.023 * g/cm3,
            2
        );

    EJ232->AddElement(C, 9);
    EJ232->AddElement(H, 10);

    std::vector<G4double> photonEnergy =
    {
        2.696 * eV,
        2.818 * eV,
        2.952 * eV,
        3.100 * eV,
        3.179 * eV,
        3.263 * eV,
        3.351 * eV,
        3.444 * eV,
        3.542 * eV,
        3.647 * eV
    };

    std::vector<G4double> scintSpectrum =
    {
        0.01,
        0.05,
        0.18,
        0.40,
        0.55,
        0.75,
        1.00,
        0.75,
        0.35,
        0.05
    };

    std::vector<G4double> rIndex =
    {
        1.58,
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

    std::vector<G4double> absLength =
    {
        10.0 * cm,
        10.0 * cm,
        10.0 * cm,
        10.0 * cm,
        10.0 * cm,
        10.0 * cm,
        10.0 * cm,
        10.0 * cm,
        10.0 * cm,
        10.0 * cm
    };

    auto* scint_mpt = new G4MaterialPropertiesTable();

    scint_mpt->AddProperty(
        "RINDEX",
        photonEnergy,
        rIndex
    );

    scint_mpt->AddProperty(
        "ABSLENGTH",
        photonEnergy,
        absLength
    );

    scint_mpt->AddProperty(
        "SCINTILLATIONCOMPONENT1",
        photonEnergy,
        scintSpectrum
    );

    scint_mpt->AddConstProperty(
        "SCINTILLATIONYIELD",
        8400. / MeV
    );

    scint_mpt->AddConstProperty(
        "SCINTILLATIONRISETIME1",
        350. * ps
    );

    scint_mpt->AddConstProperty(
        "SCINTILLATIONTIMECONSTANT1",
        1600. * ps
    );

    scint_mpt->AddConstProperty(
        "RESOLUTIONSCALE",
        1.0
    );

    EJ232->SetMaterialPropertiesTable(scint_mpt);

    return EJ232;
}


