3. Besser: nur vier Seiten mit einem Reflektor versehen

Für deinen Aufbau wäre das meiner Ansicht nach die richtige Geometrie.

Da dein Scintillator aktuell ein G4Box ist mit

G4double scint_size_x = 7 * mm;
G4double scint_size_y = 35 * mm;
G4double scint_size_z = 2 * mm;

und diese Werte bei G4Box Half-Lengths sind, sind die Ausleseseiten

$$ y=\pm35\ {\rm mm}. $$

Die Reflektoren sollten deshalb nur an

$$ x=\pm7\ {\rm mm} $$

und

$$ z=\pm2\ {\rm mm} $$

liegen.

Zum Beispiel vier dünne reflektierende Platten.

Beispiel: Reflektor in +x und -x

Nehmen wir eine Folie mit

$$ d=0.1\ {\rm mm}. $$
const G4double reflectorThickness =
    0.1 * mm;

Material kann für die geometrische Platte beispielsweise zunächst Aluminium sein:

auto* reflectorMaterial =
    nist->FindOrBuildMaterial("G4_Al");

Dann:

// ============================================================
// X-side reflectors
// ============================================================

auto* reflectorXSolid =
    new G4Box(
        "ReflectorXSolid",

        reflectorThickness / 2.0,

        scint_size_y,

        scint_size_z
    );

auto* reflectorXLogical =
    new G4LogicalVolume(
        reflectorXSolid,
        reflectorMaterial,
        "ReflectorXLogical"
    );

Positive Seite:

auto* reflectorXPlus =
    new G4PVPlacement(
        nullptr,

        G4ThreeVector(
            scint_size_x
                + reflectorThickness / 2.0,
            0,
            0
        ),

        reflectorXLogical,
        "ReflectorXPlus",

        logicWorld,
        false,
        0,
        checkOverlaps
    );

Negative Seite:

auto* reflectorXMinus =
    new G4PVPlacement(
        nullptr,

        G4ThreeVector(
            -scint_size_x
                - reflectorThickness / 2.0,
            0,
            0
        ),

        reflectorXLogical,
        "ReflectorXMinus",

        logicWorld,
        false,
        1,
        checkOverlaps
    );
4. Dasselbe für z
auto* reflectorZSolid =
    new G4Box(
        "ReflectorZSolid",

        scint_size_x,

        scint_size_y,

        reflectorThickness / 2.0
    );

auto* reflectorZLogical =
    new G4LogicalVolume(
        reflectorZSolid,
        reflectorMaterial,
        "ReflectorZLogical"
    );

Dann:

auto* reflectorZPlus =
    new G4PVPlacement(
        nullptr,

        G4ThreeVector(
            0,
            0,
            scint_size_z
                + reflectorThickness / 2.0
        ),

        reflectorZLogical,
        "ReflectorZPlus",

        logicWorld,
        false,
        0,
        checkOverlaps
    );

und:

auto* reflectorZMinus =
    new G4PVPlacement(
        nullptr,

        G4ThreeVector(
            0,
            0,
            -scint_size_z
                - reflectorThickness / 2.0
        ),

        reflectorZLogical,
        "ReflectorZMinus",

        logicWorld,
        false,
        1,
        checkOverlaps
    );

Damit bleiben bewusst:

$$ y=+35~{\rm mm} $$

und

$$ y=-35~{\rm mm} $$

frei.

5. Jetzt kommt die eigentliche optische Surface

Dazu:

#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"

Dann:

auto* reflectorSurface =
    new G4OpticalSurface(
        "ReflectorOpticalSurface"
    );

reflectorSurface->SetType(
    dielectric_metal
);

reflectorSurface->SetModel(
    unified
);

reflectorSurface->SetFinish(
    polished
);

Reflektivität:

std::vector<G4double> reflEnergy =
{
    2.6953 * eV,
    3.6466 * eV
};

std::vector<G4double> refl =
{
    0.98,
    0.98
};

Material Properties:

auto* reflectorMPT =
    new G4MaterialPropertiesTable();

reflectorMPT->AddProperty(
    "REFLECTIVITY",
    reflEnergy,
    refl
);

reflectorSurface->SetMaterialPropertiesTable(
    reflectorMPT
);

Und dann die Grenzflächen:

new G4LogicalBorderSurface(
    "ScintToXPlus",
    scintillatorPhysical,
    reflectorXPlus,
    reflectorSurface
);

new G4LogicalBorderSurface(
    "ScintToXMinus",
    scintillatorPhysical,
    reflectorXMinus,
    reflectorSurface
);

new G4LogicalBorderSurface(
    "ScintToZPlus",
    scintillatorPhysical,
    reflectorZPlus,
    reflectorSurface
);

new G4LogicalBorderSurface(
    "ScintToZMinus",
    scintillatorPhysical,
    reflectorZMinus,
    reflectorSurface
);