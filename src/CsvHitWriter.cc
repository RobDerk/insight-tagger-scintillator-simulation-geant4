#include "CsvHitWriter.hh"

#include <iomanip>

CsvHitWriter& CsvHitWriter::Instance()
{
    static CsvHitWriter instance;
    return instance;
}

bool CsvHitWriter::Open(const std::string& path)
{
    Close();

    fOut.open(path);

    if (!fOut)
    {
        return false;
    }

    // Same header layout as the Geant4 CSV ntuple output
    fOut << "#class tools::wcsv::ntuple\n"
         << "#title Optical photons reaching top or bottom detector surfaces\n"
         << "#separator 44\n"
         << "#vector_separator 59\n"
         << "#column int runID\n"
         << "#column int eventID\n"
         << "#column int trackID\n"
         << "#column string hit\n"
         << "#column double globaltime\n"
         << "#column double timesinceelectronhit\n"
         << "#column double trackLength\n"
         << "#column double photonEnergy\n"
         << "#column double photonFlightTime\n"
         << "#column double photonCreationTime\n"
         << "#column string creatorProcess\n"
         << "#column int nSideReflections\n"
         << "#column double scintLength\n"
         << "#column double emissionAngle\n"
         << "#column string material\n"
         << "#column double electronEnergy\n";

    fOut << std::setprecision(10);

    return true;
}

void CsvHitWriter::Close()
{
    if (fOut.is_open())
    {
        fOut.close();
    }
}

void CsvHitWriter::Write(const HitRecord& r)
{
    if (!fOut.is_open())
    {
        return;
    }

    fOut << r.runID << ',' << r.eventID << ',' << r.trackID << ',' << r.hit << ','
         << r.globalTime << ',' << r.timeSinceElectronHit << ',' << r.trackLength << ','
         << r.photonEnergy << ',' << r.photonFlightTime << ',' << r.photonCreationTime << ','
         << r.creatorProcess << ',' << r.nSideReflections << ',' << r.scintLength << ','
         << r.emissionAngle << ',' << r.material << ',' << r.electronEnergy << '\n';
}
