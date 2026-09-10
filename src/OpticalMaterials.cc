#include "OpticalMaterials.hh"

#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4NistManager.hh"
#include "G4OpticalSurface.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include <vector>

namespace OpticalMaterials
{
    G4Material* CreateAir()
    {
        auto* nist = G4NistManager::Instance();
        auto* air = nist->FindOrBuildMaterial("G4_AIR");

        const std::vector<G4double> photonEnergy =
        {
            1.7712 * eV,
            1.8368 * eV,
            1.9074 * eV,
            1.9837 * eV,
            2.0664 * eV,
            2.1562 * eV,
            2.2543 * eV,
            2.3616 * eV,
            2.4797 * eV,
            2.6102 * eV,
            2.6953 * eV,
            2.7552 * eV,
            2.8178 * eV,
            2.8834 * eV,
            2.9520 * eV,
            3.0240 * eV,
            3.0996 * eV,
            3.1388 * eV,
            3.1791 * eV,
            3.2204 * eV,
            3.2627 * eV,
            3.3062 * eV,
            3.3509 * eV,
            3.3968 * eV,
            3.4440 * eV,
            3.4925 * eV,
            3.5424 * eV,
            3.5937 * eV,
            3.6466 * eV,
            3.7010 * eV,
            3.7571 * eV,
            3.8149 * eV,
            3.8745 * eV,
            3.9360 * eV,
            3.9995 * eV,
            4.0651 * eV,
            4.1328 * eV
        };

        const std::vector<G4double> airRIndex(photonEnergy.size(), 1.0003);

        auto* airMPT = new G4MaterialPropertiesTable();

        airMPT->AddProperty("RINDEX", photonEnergy, airRIndex);

        air->SetMaterialPropertiesTable(airMPT);

        return air;
    }

    G4Material* CreateEJ232()
    {
        auto* nist = G4NistManager::Instance();
        auto* hydrogen = nist->FindOrBuildElement("H");
        auto* carbon = nist->FindOrBuildElement("C");

        auto* ej232 = new G4Material(
            "EJ232",
            1.023 * g / cm3, 
            2
        );

        ej232->AddElement(carbon, 9);
        ej232->AddElement(hydrogen, 10);

        const std::vector<G4double> photonEnergy =
        {
            1.7712 * eV,
            1.8368 * eV,
            1.9074 * eV,
            1.9837 * eV,
            2.0664 * eV,
            2.1562 * eV,
            2.2543 * eV,
            2.3616 * eV,
            2.4797 * eV,
            2.6102 * eV,
            2.6953 * eV,
            2.7552 * eV,
            2.8178 * eV,
            2.8834 * eV,
            2.9520 * eV,
            3.0240 * eV,
            3.0996 * eV,
            3.1388 * eV,
            3.1791 * eV,
            3.2204 * eV,
            3.2627 * eV,
            3.3062 * eV,
            3.3509 * eV,
            3.3968 * eV,
            3.4440 * eV,
            3.4925 * eV,
            3.5424 * eV,
            3.5937 * eV,
            3.6466 * eV,
            3.7010 * eV,
            3.7571 * eV,
            3.8149 * eV,
            3.8745 * eV,
            3.9360 * eV,
            3.9995 * eV,
            4.0651 * eV,
            4.1328 * eV
        };

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
            1600. * ps
        );

        scintillatorMPT->AddConstProperty(
            "SCINTILLATIONTIMECONSTANT1",
            8400. * ps
        );

        scintillatorMPT->AddConstProperty(
            "RESOLUTIONSCALE",
            1.0
        );

        ej232->SetMaterialPropertiesTable(scintillatorMPT);

        return ej232;
    }

    G4OpticalSurface* CreateIdealPhotonDetectorSurface()
    {
        auto* detectorSurface = new G4OpticalSurface("IdealPhotonCollectionSurface");
        
        detectorSurface->SetType(dielectric_metal);
        detectorSurface->SetModel(glisur);
        detectorSurface->SetFinish(polished);

        const std::vector<G4double> photonEnergy =
        {
            1.7712 * eV,
            1.8368 * eV,
            1.9074 * eV,
            1.9837 * eV,
            2.0664 * eV,
            2.1562 * eV,
            2.2543 * eV,
            2.3616 * eV,
            2.4797 * eV,
            2.6102 * eV,
            2.6953 * eV,
            2.7552 * eV,
            2.8178 * eV,
            2.8834 * eV,
            2.9520 * eV,
            3.0240 * eV,
            3.0996 * eV,
            3.1388 * eV,
            3.1791 * eV,
            3.2204 * eV,
            3.2627 * eV,
            3.3062 * eV,
            3.3509 * eV,
            3.3968 * eV,
            3.4440 * eV,
            3.4925 * eV,
            3.5424 * eV,
            3.5937 * eV,
            3.6466 * eV,
            3.7010 * eV,
            3.7571 * eV,
            3.8149 * eV,
            3.8745 * eV,
            3.9360 * eV,
            3.9995 * eV,
            4.0651 * eV,
            4.1328 * eV
        };

        const std::vector<G4double> reflectivity(photonEnergy.size(), 0.0);

        const std::vector<G4double> efficiency(photonEnergy.size(), 1.0);

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
}
