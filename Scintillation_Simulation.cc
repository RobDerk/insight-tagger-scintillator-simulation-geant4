#include <algorithm>
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
        return std::find(allowedUnits.begin(), allowedUnits.end(), unit) != allowedUnits.end();
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
            if (!ParsePositiveQuantity(token, "", {"um", "mm", "cm"}, value, unit))
            {
                G4cerr << "Invalid length (unit required: um, mm, cm): " << token << G4endl;
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

    bool ParseEnergies(const std::string& text, const std::string& defaultUnit,
                    std::vector<EnergySetting>& energies)
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
            if (!ParsePositiveQuantity(token, defaultUnit, {"eV", "keV", "MeV", "GeV", "TeV"},
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
// ./Scintillation_Simulation <macro> [events] [energies] [defaultEnergyUnit] [lengths] [materials]
// Example: ./Scintillation_Simulation run.mac 1000 '[1MeV,2MeV,1GeV]' MeV '[50mm,70mm]' '[EJ228,EJ230]'
// Also supports old scalar arguments: run.mac 1000 1 GeV '[70mm]' EJ232
// For numeric energy entries without suffix, defaultEnergyUnit applies.
// No arguments: interactive UI with vis.mac.
int main(int argc, char** argv)
{
    G4UIExecutive* ui = (argc == 1) ? new G4UIExecutive(argc, argv) : nullptr;

    G4int numberOfEvents = 1;
    std::string energiesText = "1";
    std::string defaultEnergyUnit = "GeV";
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
        if (argc >= 5) defaultEnergyUnit = argv[4];
        if (argc >= 6) scintLengthText = argv[5];
        if (argc >= 7) scintMaterialsText = argv[6];
        if (argc > 7)
        {
            G4cerr << "Too many command-line arguments" << G4endl;
            return 1;
        }
    }

    std::vector<EnergySetting> energies;
    std::vector<G4double> scintLengths;
    std::vector<std::string> scintUnits;
    std::vector<G4String> scintMaterials;

    if (!ParseEnergies(energiesText, defaultEnergyUnit, energies) ||
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

        // Build a fresh detector/run manager for each material and length.
        // This avoids requiring a SetScintMaterial() method in DetectorConstruction.
        for (const auto& material : scintMaterials)
        {
            for (std::size_t i = 0; i < scintLengths.size(); ++i)
            {
                auto* runManager = G4RunManagerFactory::CreateRunManager(
                    G4RunManagerType::SerialOnly);
                runManager->SetUserInitialization(
                    new DetectorConstruction(scintLengths[i], scintUnits[i], material));
                runManager->SetUserInitialization(new PhysicsList());
                runManager->SetUserInitialization(new ActionInitialization(true));

                if (uiManager->ApplyCommand("/control/execute " + macroFile) != 0)
                {
                    G4cerr << "Failed to execute macro: " << macroFile << G4endl;
                    delete runManager;
                    return 1;
                }

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
                delete runManager;
            }
        }
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