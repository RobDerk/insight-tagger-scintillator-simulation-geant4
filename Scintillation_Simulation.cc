#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4UnitsTable.hh"
#include "G4VisExecutive.hh"

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "OpticalMaterials.hh"
#include "PhysicsList.hh"

namespace
{
    struct EnergySetting
    {
        G4double value;
        G4String unit;
    };

    // Allow either one item ("EJ232") or a bracketed comma-separated list.
    // An empty item or list is an error.
    bool SplitArgument(
        const std::string& text,
        std::vector<std::string>& items)
    {
        items.clear();
        std::string contents = text;
        if (!contents.empty() && contents.front() == '[')
        {
            if (contents.size() < 2 || contents.back() != ']') return false;
            contents = contents.substr(1, contents.size() - 2);
        }
        else if (contents.find_first_of("[],") != std::string::npos)
        {
            return false;
        }

        std::istringstream input(contents);
        std::string token;
        while (std::getline(input, token, ','))
        {
            const auto first = token.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) return false;
            const auto last = token.find_last_not_of(" \t\r\n");
            token = token.substr(first, last - first + 1);
            if (token.find_first_of("[]") != std::string::npos) return false;
            items.push_back(token);
        }
        return !items.empty() && contents.back() != ',';
    }

    bool ParsePositiveQuantity(const std::string& token,
                           const std::string& defaultUnit,
                           const std::vector<std::string>& allowedUnits,
                           G4double& value, std::string& unit)
    {
        std::size_t pos = 0;
        try
        {
            value = std::stod(token, &pos);
        }
        catch (...)
        {
            return false;
        }

        if (!std::isfinite(value) || value <= 0.0) return false;

        unit = token.substr(pos);
        // Ignore whitespace between the value and the unit.
        unit.erase(std::remove_if(unit.begin(), unit.end(),
                                [](unsigned char c) { return c == ' ' || c == '\t'; }),
                unit.end());
        if (unit.empty()) unit = defaultUnit;

        // Case-insensitive match; continue with the canonical spelling Geant4 expects (e.g. "MeV")
        const auto toLower = [](std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return s;
        };

        const auto match = std::find_if(allowedUnits.begin(), allowedUnits.end(),
                                        [&](const std::string& allowed)
                                        { return toLower(allowed) == toLower(unit); });

        if (match == allowedUnits.end()) return false;

        unit = *match;
        return true;
    }

    bool ParseLengths(const std::string& text,
                    std::vector<G4double>& lengths,
                    std::vector<std::string>& units)
    {
        lengths.clear();
        units.clear();
        std::vector<std::string> tokens;
        if (!SplitArgument(text, tokens))
        {
            G4cerr << "Invalid lengths (example: [70mm,100mm]): " << text << G4endl;
            return false;
        }

        for (const auto& token : tokens)
        {
            G4double value = 0.0;
            std::string unit;
            if (!ParsePositiveQuantity(token, "mm", {"um", "mm", "cm"}, value, unit))
            {
                G4cerr << "Invalid length (unit: um, mm, cm; default mm): " << token << G4endl;
                return false;
            }
            const G4double length = value * G4UnitDefinition::GetValueOf(unit);
            if (length >= DetectorConstruction::kMaxScintLengthY)
            {
                G4cerr << "Scintillator length must be < "
                    << DetectorConstruction::kMaxScintLengthY / mm << " mm: " << token << G4endl;
                return false;
            }
            lengths.push_back(length);
            units.push_back(unit);
        }
        return true;
    }

    bool ParseEnergies(const std::string& text, std::vector<EnergySetting>& energies)
    {
        energies.clear();
        std::vector<std::string> tokens;
        if (!SplitArgument(text, tokens))
        {
            G4cerr << "Invalid energies (example: [1MeV,2MeV,1GeV]): " << text << G4endl;
            return false;
        }

        for (const auto& token : tokens)
        {
            G4double value = 0.0;
            std::string unit;
            if (!ParsePositiveQuantity(token, "MeV", {"eV", "keV", "MeV", "GeV", "TeV"},
                                    value, unit))
            {
                G4cerr << "Invalid energy or unit: " << token << G4endl;
                return false;
            }
            energies.push_back({value, unit});
        }
        return true;
    }

    bool ParseMaterials(const std::string& text, std::vector<G4String>& materials)
    {
        materials.clear();
        std::vector<std::string> tokens;
        if (!SplitArgument(text, tokens))
        {
            G4cerr << "Invalid materials (example: [EJ228,EJ230]): " << text << G4endl;
            return false;
        }

        for (const auto& token : tokens)
        {
            const G4String material = token;
            if (std::find(OpticalMaterials::kScintillatorNames.begin(),
                        OpticalMaterials::kScintillatorNames.end(),
                        material) == OpticalMaterials::kScintillatorNames.end())
            {
                G4cerr << "Unknown scintillator material: " << material << " (allowed:";
                for (const auto& name : OpticalMaterials::kScintillatorNames)
                    G4cerr << " " << name;
                G4cerr << ")" << G4endl;
                return false;
            }
            materials.push_back(material);
        }
        return true;
    }
} // namespace

// Batch usage:
// ./Scintillation_Simulation <macro> [events] [energies] [lengths] [materials] [--dry-run]
// --dry-run: run the full simulation but write no CSV files (accepted at any position).
// Example: ./Scintillation_Simulation run.mac 1000 '[1MeV,2MeV,1GeV]' '[50mm,70mm]' '[EJ228,EJ230]'
// Entries without a unit are MeV (energies) and mm (lengths).
// No arguments: interactive UI with vis.mac.
int main(int argc, char** argv)
{
    // Strip --dry-run so the remaining arguments stay positional
    bool dryRun = false;
    std::vector<char*> positionalArgs = {argv[0]};

    for (int a = 1; a < argc; ++a)
    {
        if (std::string(argv[a]) == "--dry-run") dryRun = true;
        else positionalArgs.push_back(argv[a]);
    }

    argc = static_cast<int>(positionalArgs.size());
    argv = positionalArgs.data();

    G4UIExecutive* ui = (argc == 1) ? new G4UIExecutive(argc, argv) : nullptr;

    G4int numberOfEvents = 1;
    std::string energiesText = "1GeV";
    std::string scintLengthText = "[70mm]";
    std::string scintMaterialsText = "EJ232";

    if (!ui)
    {
        if (argc >= 3)
        {
            try { numberOfEvents = std::stoi(argv[2]); }
            catch (...) { G4cerr << "Invalid number of events: " << argv[2] << G4endl; return 1; }
            if (numberOfEvents <= 0)
            {
                G4cerr << "Number of events must be > 0" << G4endl;
                return 1;
            }
        }
        if (argc >= 4) energiesText = argv[3];
        if (argc >= 5) scintLengthText = argv[4];
        if (argc >= 6) scintMaterialsText = argv[5];
        if (argc > 6)
        {
            G4cerr << "Too many command-line arguments" << G4endl;
            return 1;
        }
    }

    std::vector<EnergySetting> energies;
    std::vector<G4double> scintLengths;
    std::vector<std::string> scintUnits;
    std::vector<G4String> scintMaterials;

    if (!ParseEnergies(energiesText, energies) ||
        !ParseLengths(scintLengthText, scintLengths, scintUnits) ||
        !ParseMaterials(scintMaterialsText, scintMaterials))
    {
        delete ui;
        return 1;
    }

    auto* uiManager = G4UImanager::GetUIpointer();

    if (!ui)
    {
        const G4String macroFile = argv[1];
        const G4String beamOnCommand = "/run/beamOn " + std::to_string(numberOfEvents);
        const std::size_t nRuns = scintMaterials.size() * scintLengths.size() * energies.size();
        std::size_t runIndex = 0;
        const auto simulationStart = std::chrono::steady_clock::now();

        // One run manager for the whole process; the geometry is rebuilt between runs.
        auto* runManager = G4RunManagerFactory::CreateRunManager(
            G4RunManagerType::SerialOnly);

        auto* detector = new DetectorConstruction(
            scintLengths.front(), scintUnits.front(), scintMaterials.front());

        runManager->SetUserInitialization(detector);
        runManager->SetUserInitialization(new PhysicsList());
        runManager->SetUserInitialization(new ActionInitialization(!dryRun));

        // The macro contains /run/initialize, so it runs only once
        if (uiManager->ApplyCommand("/control/execute " + macroFile) != 0)
        {
            G4cerr << "Failed to execute macro: " << macroFile << G4endl;
            delete runManager;
            return 1;
        }

        bool firstGeometry = true;

        for (const auto& material : scintMaterials)
        {
            for (std::size_t i = 0; i < scintLengths.size(); ++i)
            {
                // The first combination was already built by the constructor arguments
                if (!firstGeometry)
                {
                    detector->SetScintMaterialName(material);
                    detector->SetScintLengthY(scintLengths[i], scintUnits[i]);

                    uiManager->ApplyCommand("/run/reinitializeGeometry true");
                    uiManager->ApplyCommand("/run/physicsModified");
                }

                firstGeometry = false;

                for (const auto& energy : energies)
                {
                    ++runIndex;
                    const G4String energyCommand =
                        "/gun/energy " + std::to_string(energy.value) + " " + energy.unit;
                    if (uiManager->ApplyCommand(energyCommand) != 0)
                    {
                        G4cerr << "Failed to set beam energy: " << energyCommand << G4endl;
                        delete runManager;
                        return 1;
                    }

                    G4cout << "\n===================================="
                           << "\nStarting simulation " << runIndex << "/" << nRuns
                           << "\nEvents : " << numberOfEvents
                           << "\nEnergy : " << energy.value << " " << energy.unit
                           << "\nScint length (y) : "
                           << scintLengths[i] / G4UnitDefinition::GetValueOf(scintUnits[i])
                           << " " << scintUnits[i]
                           << "\nScint material : " << material
                           << "\n====================================\n" << G4endl;

                    if (uiManager->ApplyCommand(beamOnCommand) != 0)
                    {
                        G4cerr << "BeamOn command failed" << G4endl;
                        delete runManager;
                        return 1;
                    }
                }
            }
        }

        const double totalSeconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - simulationStart).count();
        const long long total = std::llround(totalSeconds);

        G4cout << "\n===================================="
               << "\nAll simulations finished"
               << "\nRuns            : " << runIndex
               << "\nEvents per run  : " << numberOfEvents
               << "\nCSV output      : " << (dryRun ? "disabled (dry run)" : "enabled")
               << "\nTotal time      : " << total / 3600 << " h "
               << (total % 3600) / 60 << " min " << total % 60 << " s"
               << "\nMaterials       : ";

        for (std::size_t m = 0; m < scintMaterials.size(); ++m)
            G4cout << (m ? ", " : "") << scintMaterials[m];

        G4cout << "\nEnergies        : ";

        for (std::size_t e = 0; e < energies.size(); ++e)
            G4cout << (e ? ", " : "") << energies[e].value << " " << energies[e].unit;

        G4cout << "\nLengths (y)     : ";

        for (std::size_t l = 0; l < scintLengths.size(); ++l)
            G4cout << (l ? ", " : "")
                   << scintLengths[l] / G4UnitDefinition::GetValueOf(scintUnits[l])
                   << " " << scintUnits[l];

        G4cout << "\n====================================" << G4endl;

        delete runManager;
    }
    else
    {
        auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);
        runManager->SetUserInitialization(
            new DetectorConstruction(scintLengths.front(), scintUnits.front(), scintMaterials.front()));
        runManager->SetUserInitialization(new PhysicsList());
        runManager->SetUserInitialization(new ActionInitialization(false));

        auto* visManager = new G4VisExecutive(argc, argv);
        visManager->Initialize();
        uiManager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete visManager;
        delete runManager;
        delete ui;
    }
    return 0;
}