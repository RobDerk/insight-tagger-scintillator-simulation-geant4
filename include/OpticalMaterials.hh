#ifndef OpticalMaterials_hh
#define OpticalMaterials_hh 1

#include "globals.hh"

class G4Material;
class G4OpticalSurface;

namespace OpticalMaterials
{
    // Materials
    G4Material* CreateAir();
    G4Material* CreateEJ232();
    G4Material* Saphir_Glass();

    // PDE file format: two columns, wavelength [nm] and PDE as fraction 0..1
    // PDE for the AFBR-S4N44P014M NUV-MT Silicon Photomultiplier
    G4OpticalSurface* CreateIdealPhotonDetectorSurface(const G4String& pdeFile);
}

#endif
