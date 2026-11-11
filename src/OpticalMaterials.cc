#include "OpticalMaterials.hh"

#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4NistManager.hh"
#include "G4OpticalSurface.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include "G4Exception.hh"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace
{
    // Returns energy-ascending vectors, as required by G4MaterialPropertyVector
    bool LoadPDE(const G4String& path,
                 std::vector<G4double>& energy,
                 std::vector<G4double>& pde)
    {
        std::ifstream in(path);

        if (!in)
        {
            return false;
        }

        std::string line;
        std::getline(in, line); // header

        while (std::getline(in, line))
        {
            std::istringstream iss(line);
            G4double lambdaNm = 0.0;
            G4double value = 0.0;

            if (iss >> lambdaNm >> value && lambdaNm > 0.0)
            {
                energy.push_back(h_Planck * c_light / (lambdaNm * nm));
                pde.push_back(value);
            }
        }

        std::reverse(energy.begin(), energy.end());
        std::reverse(pde.begin(), pde.end());

        return !energy.empty();
    }

    // Polyvinyltoluene base (C9H10, 1.023 g/cm3) shared by the EJ scintillators
    G4Material* CreatePVTBase(const G4String& name)
    {
        auto* nist = G4NistManager::Instance();

        auto* material = new G4Material(name, 1.023 * g / cm3, 2);
        material->AddElement(nist->FindOrBuildElement("C"), 9);
        material->AddElement(nist->FindOrBuildElement("H"), 10);

        return material;
    }
}

namespace OpticalMaterials
{
    G4Material* CreateAir()
    {
        auto* nist = G4NistManager::Instance();
        auto* air = nist->FindOrBuildMaterial("G4_AIR");

        const auto& photonEnergy = kScintillatorPhotonEnergy;

        const std::vector<G4double> airRIndex(photonEnergy.size(), 1.0003);

        auto* airMPT = new G4MaterialPropertiesTable();

        airMPT->AddProperty("RINDEX", photonEnergy, airRIndex);

        air->SetMaterialPropertiesTable(airMPT);

        return air;
    }

    G4Material* CreateEJ232()
    {
        if (auto* existing = G4Material::GetMaterial("EJ232", false))
        {
            return existing;
        }

        auto* ej232 = CreatePVTBase("EJ232");

        const auto& photonEnergy = kScintillatorPhotonEnergy;

        // Values outside 340-460 nm are extrapolated, not manufacturer data.
        const std::vector<G4double> scintSpectrum =
        {
            0.00002,
            0.00003,
            0.0001,
            0.0002,
            0.0004,
            0.001,
            0.002,
            0.005,
            0.01,
            0.02,
            0.03,
            0.06,
            0.10,
            0.18,
            0.30,
            0.39,
            0.52,
            0.60,
            0.75,
            0.84,
            0.82,
            0.87,
            1.00,
            0.90,
            0.68,
            0.50,
            0.52,
            0.35,
            0.12,
            0.08,
            0.05,
            0.03,
            0.018,
            0.010,
            0.006,
            0.003,
            0.0015
        };

        const std::vector<G4double> rIndex(photonEnergy.size(), 1.58);

        // UV absorption values below 340 nm are extrapolated estimates.
        const std::vector<G4double> absLength =
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
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            5.0 * cm,
            2.0 * cm,
            0.5 * cm,
            0.1 * cm,
            0.02 * cm,
            0.005 * cm,
            0.001 * cm,
            0.0005 * cm
        };

        auto* scintillatorMPT = new G4MaterialPropertiesTable();

        scintillatorMPT->AddProperty(
            "RINDEX",
            photonEnergy,
            rIndex
        );

        scintillatorMPT->AddProperty(
            "ABSLENGTH",
            photonEnergy,
            absLength
        );

        scintillatorMPT->AddProperty(
            "SCINTILLATIONCOMPONENT1",
            photonEnergy,
            scintSpectrum
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONYIELD",
            8400. / MeV
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONRISETIME1",
            350. * ps
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONTIMECONSTANT1",
            1600. * ps
        );

        scintillatorMPT->AddConstProperty(
            "RESOLUTIONSCALE",
            1.0
        );

        ej232->SetMaterialPropertiesTable(scintillatorMPT);

        return ej232;
    }

    G4Material* CreateEJ232Q05()
    {
        if (auto* existing = G4Material::GetMaterial("EJ232Q05", false))
        {
            return existing;
        }

        auto* ej232Q05 = CreatePVTBase("EJ232Q05");

        const auto& photonEnergy = kScintillatorPhotonEnergy;

        // Values outside 340-460 nm are extrapolated, not manufacturer data.
        const std::vector<G4double> scintSpectrum =
        {
            0.00002,
            0.00003,
            0.0001,
            0.0002,
            0.0004,
            0.001,
            0.002,
            0.005,
            0.01,
            0.02,
            0.03,
            0.06,
            0.10,
            0.18,
            0.30,
            0.39,
            0.52,
            0.60,
            0.75,
            0.84,
            0.82,
            0.87,
            1.00,
            0.90,
            0.68,
            0.50,
            0.52,
            0.35,
            0.12,
            0.08,
            0.05,
            0.03,
            0.018,
            0.010,
            0.006,
            0.003,
            0.0015
        };

        const std::vector<G4double> rIndex(photonEnergy.size(), 1.58);

        // UV absorption values below 340 nm are extrapolated estimates.
        const std::vector<G4double> absLength =
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
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            10.0 * cm,
            5.0 * cm,
            2.0 * cm,
            0.5 * cm,
            0.1 * cm,
            0.02 * cm,
            0.005 * cm,
            0.001 * cm,
            0.0005 * cm
        };

        auto* scintillatorMPT = new G4MaterialPropertiesTable();

        scintillatorMPT->AddProperty(
            "RINDEX",
            photonEnergy,
            rIndex
        );

        scintillatorMPT->AddProperty(
            "ABSLENGTH",
            photonEnergy,
            absLength
        );

        scintillatorMPT->AddProperty(
            "SCINTILLATIONCOMPONENT1",
            photonEnergy,
            scintSpectrum
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONYIELD",
            2900. / MeV
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONRISETIME1",
            110. * ps
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONTIMECONSTANT1",
            700. * ps
        );

        scintillatorMPT->AddConstProperty(
            "RESOLUTIONSCALE",
            1.0
        );

        ej232Q05->SetMaterialPropertiesTable(scintillatorMPT);

        return ej232Q05;
    }

    G4Material* CreateEJ230()
    {
        if (auto* existing = G4Material::GetMaterial("EJ230", false))
        {
            return existing;
        }

        auto* ej230 = CreatePVTBase("EJ230");

        const auto& photonEnergy = kScintillatorPhotonEnergy;

        // Read off the Eljen EJ-228/EJ-230 emission plot (peak 391 nm), not tabulated data.
        const std::vector<G4double> scintSpectrum =
        {
            0.0,    // 700 nm
            0.0,    // 675 nm
            0.0,    // 650 nm
            0.0,    // 625 nm
            0.0,    // 600 nm
            0.0,    // 575 nm
            0.0,    // 550 nm
            0.0,    // 525 nm
            0.005,  // 500 nm
            0.04,   // 475 nm
            0.07,   // 460 nm
            0.12,   // 450 nm
            0.22,   // 440 nm
            0.31,   // 430 nm
            0.45,   // 420 nm
            0.64,   // 410 nm
            0.88,   // 400 nm
            0.96,   // 395 nm
            1.00,   // 390 nm
            0.95,   // 385 nm
            0.84,   // 380 nm
            0.55,   // 375 nm
            0.22,   // 370 nm
            0.10,   // 365 nm
            0.05,   // 360 nm
            0.02,   // 355 nm
            0.0,    // 350 nm
            0.0,    // 345 nm
            0.0,    // 340 nm
            0.0,    // 335 nm
            0.0,    // 330 nm
            0.0,    // 325 nm
            0.0,    // 320 nm
            0.0,    // 315 nm
            0.0,    // 310 nm
            0.0,    // 305 nm
            0.0     // 300 nm
        };

        const std::vector<G4double> rIndex(photonEnergy.size(), 1.58);

        // 120 cm is the datasheet value; the UV edge below 380 nm is an estimate.
        const std::vector<G4double> absLength =
        {
            120.0 * cm,   // 700 nm
            120.0 * cm,   // 675 nm
            120.0 * cm,   // 650 nm
            120.0 * cm,   // 625 nm
            120.0 * cm,   // 600 nm
            120.0 * cm,   // 575 nm
            120.0 * cm,   // 550 nm
            120.0 * cm,   // 525 nm
            120.0 * cm,   // 500 nm
            120.0 * cm,   // 475 nm
            120.0 * cm,   // 460 nm
            120.0 * cm,   // 450 nm
            120.0 * cm,   // 440 nm
            120.0 * cm,   // 430 nm
            120.0 * cm,   // 420 nm
            120.0 * cm,   // 410 nm
            120.0 * cm,   // 400 nm
            120.0 * cm,   // 395 nm
            120.0 * cm,   // 390 nm
            120.0 * cm,   // 385 nm
            120.0 * cm,   // 380 nm
            60.0 * cm,    // 375 nm
            20.0 * cm,    // 370 nm
            5.0 * cm,     // 365 nm
            1.0 * cm,     // 360 nm
            0.2 * cm,     // 355 nm
            0.05 * cm,    // 350 nm
            0.01 * cm,    // 345 nm
            0.005 * cm,   // 340 nm
            0.002 * cm,   // 335 nm
            0.001 * cm,   // 330 nm
            0.0005 * cm,  // 325 nm
            0.0005 * cm,  // 320 nm
            0.0005 * cm,  // 315 nm
            0.0005 * cm,  // 310 nm
            0.0005 * cm,  // 305 nm
            0.0005 * cm   // 300 nm
        };

        auto* scintillatorMPT = new G4MaterialPropertiesTable();

        scintillatorMPT->AddProperty(
            "RINDEX",
            photonEnergy,
            rIndex
        );

        scintillatorMPT->AddProperty(
            "ABSLENGTH",
            photonEnergy,
            absLength
        );

        scintillatorMPT->AddProperty(
            "SCINTILLATIONCOMPONENT1",
            photonEnergy,
            scintSpectrum
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONYIELD",
            9700. / MeV
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONRISETIME1",
            500. * ps
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONTIMECONSTANT1",
            1500. * ps
        );

        scintillatorMPT->AddConstProperty(
            "RESOLUTIONSCALE",
            1.0
        );

        ej230->SetMaterialPropertiesTable(scintillatorMPT);

        return ej230;
    }

    G4Material* CreateScintillator(const G4String& name)
    {
        if (name == "EJ232") return CreateEJ232();
        if (name == "EJ232Q05") return CreateEJ232Q05();
        if (name == "EJ230") return CreateEJ230();

        return nullptr;
    }

    G4OpticalSurface* CreateIdealPhotonDetectorSurface(const G4String& pdeFile)
    {
        auto* detectorSurface = new G4OpticalSurface("IdealPhotonCollectionSurface");
        
        detectorSurface->SetType(dielectric_metal);
        detectorSurface->SetModel(glisur);
        detectorSurface->SetFinish(polished);

        std::vector<G4double> photonEnergy;
        std::vector<G4double> efficiency;

        if (!LoadPDE(pdeFile, photonEnergy, efficiency))
        {
            G4Exception(
                "OpticalMaterials::CreateIdealPhotonDetectorSurface",
                "PDE001",
                FatalException,
                ("Cannot read PDE file: " + pdeFile).c_str()
            );
        }

        const std::vector<G4double> reflectivity(photonEnergy.size(), 0.0);

        auto* detectorMPT = new G4MaterialPropertiesTable();

        detectorMPT->AddProperty(
            "REFLECTIVITY",
            photonEnergy,
            reflectivity
        );

        detectorMPT->AddProperty(
            "EFFICIENCY",
            photonEnergy,
            efficiency
        );
        
        detectorSurface->SetMaterialPropertiesTable(detectorMPT);

        return detectorSurface;
    }

    G4Material* Saphir_Glass()
    {
        auto* nist = G4NistManager::Instance();
        auto* silicon = nist->FindOrBuildElement("Si");
        auto* oxygen = nist->FindOrBuildElement("O");

        auto* saphirGlass = new G4Material("Saphir_Glass", 2.2 * g / cm3, 2);
        saphirGlass->AddElement(silicon, 1);
        saphirGlass->AddElement(oxygen, 2);

        const auto& photonEnergy = kSapphirePhotonEnergy;

        const std::vector<G4double> refractiveIndex(photonEnergy.size(), 1.77);

        const std::vector<G4double> absLength(photonEnergy.size(), 5.0 * m);

        auto* saphirGlassMPT = new G4MaterialPropertiesTable();

        saphirGlassMPT->AddProperty(
            "RINDEX",
            photonEnergy,
            refractiveIndex
        );
        
        saphirGlassMPT->AddProperty(
            "ABSLENGTH",
            photonEnergy,
            absLength
        );

        saphirGlass->SetMaterialPropertiesTable(saphirGlassMPT);

        return saphirGlass;

    }

    G4OpticalSurface* CreateReflectorSurface()
    {
        auto* reflectorSurface = new G4OpticalSurface("ReflectorSurface");
        auto* relflectorMPT = new G4MaterialPropertiesTable();

        std::vector<G4double> photonEnergy = {
            1.4 * eV,
            4.5 * eV
        };

        std::vector<G4double> reflectivity = {
            0.90,
            0.90
        };

        relflectorMPT->AddProperty(
            "REFLECTIVITY",
            photonEnergy,
            reflectivity
        );

        relflectorMPT->AddProperty(
            "EFFICIENCY",
            photonEnergy,
            std::vector<G4double>(photonEnergy.size(), 0.0)
        );

        reflectorSurface->SetMaterialPropertiesTable(relflectorMPT);

        reflectorSurface->SetType(dielectric_metal);
        reflectorSurface->SetModel(glisur);
        reflectorSurface->SetFinish(polished);

        return reflectorSurface;
    }
}
