#ifndef OpticalMaterials_hh
#define OpticalMaterials_hh 1

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"

#include <vector>

class G4Material;
class G4OpticalSurface;

namespace OpticalMaterials
{
    // Materials
    G4Material* CreateAir();
    
    // Scintillator material
    G4Material* CreateEJ232();
    G4Material* CreateEJ232Q05();
    G4Material* CreateEJ230();

    // Reflector surface material
    G4OpticalSurface* CreateReflectorSurface();

    // Names accepted by CreateScintillator
    inline const std::vector<G4String> kScintillatorNames = {"EJ232", "EJ232Q05", "EJ230"};

    // Returns nullptr for a name not in kScintillatorNames
    G4Material* CreateScintillator(const G4String& name);
    
    G4Material* Saphir_Glass();

    // PDE file format: two columns, wavelength [nm] and PDE as fraction 0..1
    // PDE for the AFBR-S4N44P014M NUV-MT Silicon Photomultiplier
    G4OpticalSurface* CreateIdealPhotonDetectorSurface(const G4String& pdeFile);

    // Photon energy grid for air and the EJ scintillators (700 to 300 nm), ascending in energy
    inline const std::vector<G4double> kScintillatorPhotonEnergy =
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

    // Photon energy grid for sapphire glass (700 to 200 nm); it stays transparent well into the UV
    inline const std::vector<G4double> kSapphirePhotonEnergy =
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
        4.1328 * eV,
        4.2028 * eV,
        4.2753 * eV,
        4.3503 * eV,
        4.4280 * eV,
        4.5085 * eV,
        4.5920 * eV,
        4.6786 * eV,
        4.7686 * eV,
        4.8621 * eV,
        4.9594 * eV,
        5.0606 * eV,
        5.1660 * eV,
        5.2759 * eV,
        5.3906 * eV,
        5.5104 * eV,
        5.6356 * eV,
        5.7667 * eV,
        5.9040 * eV,
        6.0480 * eV,
        6.1992 * eV
    };
}

#endif
