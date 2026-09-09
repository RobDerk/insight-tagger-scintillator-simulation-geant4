#include <iostream>
#include <string>

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"


int main(int argc, char** argv)
{
    G4UIExecutive* ui = nullptr;

    if (argc == 1)
    {
        ui = new G4UIExecutive(argc, argv);
    }


    // =========================================================
    // Run Manager
    // =========================================================

    auto runManager =
        G4RunManagerFactory::CreateRunManager(
            G4RunManagerType::SerialOnly
        );


    runManager->SetUserInitialization(
        new DetectorConstruction()
    );

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

        // Default values

        G4int numberOfEvents = 1;
        G4double energyValue = 1.0;
        G4String energyUnit = "GeV";

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

                delete runManager;
                return 1;
            }


            if (numberOfEvents <= 0)
            {
                G4cerr
                    << "Number of events must be > 0"
                    << G4endl;

                delete runManager;
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

                delete runManager;
                return 1;
            }


            if (energyValue <= 0.0)
            {
                G4cerr
                    << "Energy must be > 0"
                    << G4endl;

                delete runManager;
                return 1;
            }
        }

        if (argc >= 5)
        {
            energyUnit = argv[4];
        }

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

        // Start simulation

        const G4String beamOnCommand =
            "/run/beamOn "
            + std::to_string(numberOfEvents);


        G4cout
            << "\n===================================="
            << "\nStarting simulation"
            << "\nEvents : "
            << numberOfEvents
            << "\nEnergy : "
            << energyValue
            << " "
            << energyUnit
            << "\n====================================\n"
            << G4endl;


        uiManager->ApplyCommand(
            beamOnCommand
        );
    }

    // UI Mode
    else
    {
        auto visManager =
            new G4VisExecutive(argc, argv);

        visManager->Initialize();


        uiManager->ApplyCommand(
            "/control/execute init_vis.mac"
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