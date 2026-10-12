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

    G4OpticalSurface* CreateIdealPhotonDetectorSurface();
}

#endif
