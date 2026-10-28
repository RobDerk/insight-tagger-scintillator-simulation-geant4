#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"


// Parses "70", "100,70,50" or "[100mm,7cm]"; entries without unit use defaultUnit.
// The world has a half-size of 100 mm; the end-face collectors must fit inside.
static bool ParseLengths(
    const std::string& text,
    const std::string& defaultUnit,
    std::vector<G4double>& lengths)
{
    std::string cleaned;

    for (const char c : text)
    {
        if (c != '[' && c != ']' && c != ' ')
        {
            cleaned += c;
        }
    }

    std::istringstream stream(cleaned);
    std::string token;

    while (std::getline(stream, token, ','))
    {
        std::size_t pos = 0;
        G4double value = 0.0;

        try
        {
            value = std::stod(token, &pos);
        }
        catch (...)
        {
            G4cerr << "Invalid scintillator length: " << token << G4endl;
            return false;
        }

        std::string unit = token.substr(pos);

        if (unit.empty())
        {
            unit = defaultUnit;
        }

        G4double unitFactor = 0.0;

        if (unit == "um") unitFactor = um;
        else if (unit == "mm") unitFactor = mm;
        else if (unit == "cm") unitFactor = cm;
        else
        {
            G4cerr << "Invalid length unit: " << unit << " (use um, mm or cm)" << G4endl;
            return false;
        }

        const G4double length = value * unitFactor;

        if (length <= 0.0 || length >= 198.0 * mm)
        {
            G4cerr << "Scintillator length must be > 0 and < 198 mm: " << token << G4endl;
            return false;
        }

        lengths.push_back(length);
    }

    if (lengths.empty())
    {
        G4cerr << "No scintillator length given" << G4endl;
        return false;
    }

    return true;
}

// Usage (batch): ./Scintillation_Simulation <macro> [events] [energy] [energyUnit] [lengths] [lengthUnit]
// lengths: single value or comma separated list, e.g. 70 or 100,70,50 or "[100mm,70mm]"; one run per length.
// No arguments: interactive UI mode with vis.mac.
int main(int argc, char** argv)
{
    G4UIExecutive* ui = nullptr;

    if (argc == 1)
    {
        ui = new G4UIExecutive(argc, argv);
    }

    // Defaults
    G4int numberOfEvents = 1;
    G4double energyValue = 1.0;
    G4String energyUnit = "GeV";

    // Full lengths of the scintillator along y, one run per entry
    std::string scintLengthText = "70";
    std::string scintLengthUnit = "mm";
    std::vector<G4double> scintLengths = {70.0 * mm};

    // =========================================================
    // Command line arguments (batch mode)
    // =========================================================

    if (!ui)
    {
        if (argc >= 3)
        {
            try
            {
                numberOfEvents =
                    std::stoi(argv[2]);
            }
            catch (...)
            {
                G4cerr
                    << "Invalid number of events: "
                    << argv[2]
                    << G4endl;

                return 1;
            }


            if (numberOfEvents <= 0)
            {
                G4cerr
                    << "Number of events must be > 0"
                    << G4endl;

                return 1;
            }
        }

        if (argc >= 4)
        {
            try
            {
                energyValue =
                    std::stod(argv[3]);
            }
            catch (...)
            {
                G4cerr
                    << "Invalid energy value: "
                    << argv[3]
                    << G4endl;

                return 1;
            }


            if (energyValue <= 0.0)
            {
                G4cerr
                    << "Energy must be > 0"
                    << G4endl;

                return 1;
            }
        }

        if (argc >= 5)
        {
            energyUnit = argv[4];
        }

        if (argc >= 6)
        {
            scintLengthText = argv[5];
        }

        if (argc >= 7)
        {
            scintLengthUnit = argv[6];
        }

        scintLengths.clear();

        if (!ParseLengths(scintLengthText, scintLengthUnit, scintLengths))
        {
            return 1;
        }
    }


    // =========================================================
    // Run Manager
    // =========================================================

    auto runManager =
        G4RunManagerFactory::CreateRunManager(
            G4RunManagerType::SerialOnly
        );


    auto detector = new DetectorConstruction(scintLengths.front());

    runManager->SetUserInitialization(detector);

    runManager->SetUserInitialization(
        new PhysicsList()
    );

    runManager->SetUserInitialization(
        new ActionInitialization()
    );


    auto uiManager =
        G4UImanager::GetUIpointer();


    // =========================================================
    // BATCH MODE
    // =========================================================

    if (!ui)
    {
        G4String macroFile = argv[1];

        uiManager->ApplyCommand(
            "/control/execute " + macroFile
        );

        // set energy of particle

        const G4String energyCommand =
            "/gun/energy "
            + std::to_string(energyValue)
            + " "
            + energyUnit;


        G4cout
            << "\nSetting beam energy: "
            << energyValue
            << " "
            << energyUnit
            << G4endl;


        uiManager->ApplyCommand(
            energyCommand
        );

        // Start simulation: one run per scintillator length

        const G4String beamOnCommand =
            "/run/beamOn "
            + std::to_string(numberOfEvents);

        for (std::size_t i = 0; i < scintLengths.size(); ++i)
        {
            if (i > 0)
            {
                // Geometry is rebuilt with the new length
                detector->SetScintLengthY(scintLengths[i]);

                uiManager->ApplyCommand("/run/reinitialize");
            }

            G4cout
                << "\n===================================="
                << "\nStarting simulation " << (i + 1) << "/" << scintLengths.size()
                << "\nEvents : "
                << numberOfEvents
                << "\nEnergy : "
                << energyValue
                << " "
                << energyUnit
                << "\nScint length (y) : "
                << scintLengths[i] / mm
                << " mm"
                << "\n====================================\n"
                << G4endl;

            uiManager->ApplyCommand(
                beamOnCommand
            );
        }
    }

    // UI Mode
    else
    {
        auto visManager =
            new G4VisExecutive(argc, argv);

        visManager->Initialize();


        uiManager->ApplyCommand(
            "/control/execute vis.mac"
        );

        ui->SessionStart();


        G4cout
            << "Starting simulation in UI mode"
            << G4endl;


        delete visManager;
        delete ui;
    }


    delete runManager;

    return 0;
}