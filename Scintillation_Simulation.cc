#include <iostream>

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

    if (argc == 1) {
      ui = new G4UIExecutive(argc, argv);
    }

    // construct the default run Manager
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
    
    runManager->SetUserInitialization(new DetectorConstruction());
    runManager->SetUserInitialization(new PhysicsList());
    runManager->SetUserInitialization(new ActionInitialization());

    // construct the vis
    auto visManager = new G4VisExecutive(argc, argv);
    
    visManager->Initialize();

    // User Interface manager
    auto uiManager = G4UImanager::GetUIpointer();
    
    // Process macro or start UI session
    //
    if (!ui) {
        // batch mode
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        uiManager->ApplyCommand(command + fileName);
    }
    else {
        // interactive mode
        uiManager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
        delete ui;
    }

    delete visManager;
    delete runManager;

    return 0;
}