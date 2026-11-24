#ifndef CsvHitWriter_hh
#define CsvHitWriter_hh 1

#include "globals.hh"

#include <fstream>
#include <string>

// One detected photon; values are already in the output units (ns, mm, eV, deg, MeV)
struct HitRecord
{
    G4int runID = 0;
    G4int eventID = 0;
    G4int trackID = 0;
    std::string hit;
    G4double globalTime = 0.0;
    G4double timeSinceElectronHit = 0.0;
    G4double trackLength = 0.0;
    G4double photonEnergy = 0.0;
    G4double photonFlightTime = 0.0;
    G4double photonCreationTime = 0.0;
    std::string creatorProcess;
    G4int nSideReflections = 0;
    G4double scintLength = 0.0;
    G4double emissionAngle = 0.0;
    std::string material;
    G4double electronEnergy = 0.0;
};

// Writes the hits as CSV next to the ROOT ntuple. The analysis manager can only
// write one file type per run, so the CSV is written separately.
class CsvHitWriter
{
  public:
    static CsvHitWriter& Instance();

    bool Open(const std::string& path);
    void Close();
    void Write(const HitRecord& record);

  private:
    CsvHitWriter() = default;

    std::ofstream fOut;
};

#endif
