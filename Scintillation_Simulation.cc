#include <iostream>

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"

#include <string>

int main(int argc, char** argv)
{
    G4UIExecutive* ui = nullptr;

    if (argc == 1)
    {
      ui = new G4UIExecutive(argc, argv);
    }

    // construct the default run Manager
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);
    
    runManager->SetUserInitialization(new DetectorConstruction());
    runManager->SetUserInitialization(new PhysicsList());
    runManager->SetUserInitialization(new ActionInitialization());

    // User Interface manager
    auto uiManager = G4UImanager::GetUIpointer();
    
    // Process macro or start UI session

    // batch mode
    if (!ui)
    {
        // Macro file
        G4String macroFile = argv[1];
    
        // Default number of events
        G4int numberOfEvents = 1;
    
        // Optional second command line argument
        if (argc >= 3)
        {
            try
            {
                numberOfEvents = std::stoi(argv[2]);
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
    
        // Execute configuration macro
        uiManager->ApplyCommand(
            "/control/execute " + macroFile
        );
    
        // Start simulation
        G4String beamOnCommand =
            "/run/beamOn "
            + std::to_string(numberOfEvents);
    
        G4cout
            << "\nStarting simulation with "
            << numberOfEvents
            << " events\n"
            << G4endl;
    
        uiManager->ApplyCommand(
            beamOnCommand
        );
    }
    // interactive ui mode
    else
    {
        auto visManager = new G4VisExecutive(argc, argv);

        visManager->Initialize();

        uiManager->ApplyCommand("/control/execute init_vis.mac");

        ui->SessionStart();

        G4cout << "Starting simulation in ui mode" << G4endl;

        delete visManager;
        delete ui;
    }

    delete runManager;

    return 0;
}