# Scintillation Simulation

Geant4-Simulation eines Plastikszintillator, der von einem Elektronenstrahl getroffen wird. Erzeugte optische Photonen (Szintillation und Cherenkov) werden im Szintillator transportiert und an den beiden Flächen (oben / unten) von "idealen Photonendetektoren" gezählt.

Das Szintillatormaterial ist austauschbar. Aktuell ist EJ-232 hinterlegt; weitere Materialien werden über `OpticalMaterials` ergänzt.

## Aufbau

| Datei | Aufgabe |
|---|---|
| `Scintillation_Simulation.cc` | `main()`, Kommandozeilenargumente, Batch-/UI-Modus |
| `DetectorConstruction` | Geometrie: World (Luft), Szintillator, Photonen-Kollektoren oben/unten |
| `OpticalMaterials` | Materialdefinitionen und optische Eigenschaften (Brechungsindex, Absorption, Szintillationsspektrum, Lichtausbeute, Oberflächen) |
| `PhysicsList` | Physikmodelle, physikalische Prozesse |
| `PrimaryGeneratorAction` | Teilchenquelle (Standard: e-, +z-Richtung) |
| `SteppingAction` | Photonen-Tracking, Zählung an den Stirnflächen, Reflexionen, Eintrittszeit des Elektrons |
| `EventAction` | Zähler pro Event, Ausgabe der Event-Zusammenfassung |
| `RunAction` | CSV-Ausgabe, Zusammenfassung pro Run |
| `ScintillatorSD`, `PMTSD` | Sensitive Detektoren |

## Geometrie (Standardwerte)

Die Geometrie-Maße in der Tabelle sind Halblängen (Geant4 `G4Box`). Die Szintillatorlänge wird auf der Kommandozeile als volle Länge angegeben und intern halbiert.

| Größe | Wert |
|---|---|
| Szintillator x (Breite) | 7 mm |
| Szintillator y (Länge, Richtung der Photonenkollektoren) | 35 mm |
| Szintillator z (Dicke) | 2 mm |
| World | 100 mm in jede Richtung |

Die Photonen-Kollektoren sitzen an den beiden y-Enden des Szintillators.

## Voraussetzungen

- Geant4 (mit `ui_all` und `vis_all`, optische Physik, Analysis/CSV)
- CMake >= 3.16
- C++17-Compiler

## Bauen

Geant4-Umgebung laden (Pfad anpassen), dann bauen:

```bash
source /pfad/zu/geant4-install/bin/geant4.sh
mkdir build && cd build
cmake ..
make -j
```

## Ausführen

### Interaktiver Modus (Visualisierung)

```bash
./Scintillation_Simulation
```

Führt `vis.mac` aus (OpenGL-Viewer, 1 Event).

### Batch-Modus

```bash
./Scintillation_Simulation <macro> [events] [energie] [energieeinheit] [längen]
```

| Argument | Bedeutung | Standard |
|---|---|---|
| `macro` | Makrodatei, z. B. `run.mac` | (Pflicht) |
| `events` | Anzahl Events | `1` |
| `energie` | Energie des Primärteilchens | `1` |
| `energieeinheit` | Geant4-Energieeinheit (`eV`, `keV`, `MeV`, `GeV`, ...) | `GeV` |
| `längen` | Volle Länge des Szintillators in y als Liste in Anführungszeichen, z. B. `"[70mm,100mm]"`; Einheit (`um`, `mm`, `cm`) pro Eintrag Pflicht, pro Länge ein Run | `"[70mm]"` |

Jede Länge muss größer als 0 und kleiner als 198 mm sein (Größe der World).

Beispiele:

```bash
# Standardwerte: 1 Event, 1 GeV, 70 mm
./Scintillation_Simulation run.mac

# 1000 Events mit 500 MeV
./Scintillation_Simulation run.mac 1000 500 MeV

# 1000 Events, 1 GeV, Szintillatorlänge 50 mm
./Scintillation_Simulation run.mac 1000 1 GeV "[50mm]"

# Mehrere Längen: je ein Run mit 100, 70 und 50 mm
./Scintillation_Simulation run.mac 1000 1 GeV "[100mm,70mm,50mm]"
```

## Ausgabe

- Konsole: pro Event Primärteilchen, Energiedeposition, Zeit des Elektroneneintritts, Photonenzahlen oben/unten/gesamt, Asymmetrie und Rechenzeit. Am Ende jedes Runs Gesamt- und Zeit pro Event.
- Dateien: CSV-Dateien im Ordner `data/` (`<länge>mm.csv`, z. B. `70mm.csv`; gleiche Länge überschreibt die Datei) mit je einem Eintrag pro Photon-Treffer:

| Spalte | Inhalt |
|---|---|
| `runID`, `eventID`, `trackID` | Zuordnung |
| `hit` | Top oder Bottom |
| `globaltime` | Globale Zeit des Treffers |
| `timesinceelectronhit` | Zeit seit Eintritt des Elektrons in den Szintillator |
| `trackLength` | Spurlänge des Photons |
| `photonEnergy` | Photonenenergie |
| `photonFlightTime` | Flugzeit des Photons |
| `photonCreationTime` | Erzeugungszeit des Photons |
| `creatorProcess` | Erzeugungsprozess (z. B. Scintillation, Cerenkov) |
| `nSideReflections` | Anzahl Reflexionen an den Seitenflächen |

## Anderes Szintillatormaterial verwenden

1. In `include/OpticalMaterials.hh` und `src/OpticalMaterials.cc` eine neue Funktion anlegen, z. B. `G4Material* CreateEJ200();`, mit Zusammensetzung, Dichte und `G4MaterialPropertiesTable` (`RINDEX`, `ABSLENGTH`, `SCINTILLATIONCOMPONENT1`, `SCINTILLATIONYIELD`, `RESOLUTIONSCALE`, `SCINTILLATIONTIMECONSTANT1`, ...).
2. In `DetectorConstruction::Construct()` die Zeile `scintillator_material = OpticalMaterials::CreateEJ232();` auf die neue Funktion umstellen.
3. Neu bauen.

## Makros

- `run.mac`: Batch-Einstellungen (`/run/initialize`, Verbosity, Teilchen, Energie, Richtung)
- `vis.mac`: Visualisierung und Trajektorien; enthält auskommentierte Filter und Einfallswinkel-Beispiele
