# Scintillation Simulation

Geant4-Simulation eines Plastikszintillators, der von einem Elektronenstrahl getroffen wird. Die erzeugten optischen Photonen (Szintillation und Cherenkov) werden im Szintillator transportiert und an den beiden Stirnflächen (oben/unten) von „idealen Photonendetektoren“ gezählt.

Das Szintillatormaterial ist austauschbar. Im **Batch-Modus** können jetzt **mehrere Energien, Szintillatorlängen und Materialien** in einem Aufruf angegeben werden. Die Simulation führt **alle Kombinationen** dieser drei Listen aus.

## Aufbau

| Datei | Aufgabe |
|---|---|
| `Scintillation_Simulation.cc` | `main()`, Kommandozeilenargumente, Parsing der Listen, Batch-/UI-Modus und Simulationsschleifen |
| `DetectorConstruction` | Geometrie: World (Luft), Szintillator, Photonen-Kollektoren oben/unten |
| `OpticalMaterials` | Materialdefinitionen und optische Eigenschaften (Brechungsindex, Absorption, Szintillationsspektrum, Lichtausbeute, Oberflächen) |
| `PhysicsList` | Physikmodelle, physikalische Prozesse |
| `PrimaryGeneratorAction` | Teilchenquelle (Standard: e⁻, +z-Richtung) |
| `SteppingAction` | Photonen-Tracking, Zählung an den Stirnflächen, Reflexionen, Eintrittszeit des Elektrons |
| `EventAction` | Zähler pro Event, Ausgabe der Event-Zusammenfassung |
| `RunAction` | CSV-Ausgabe, Zusammenfassung pro Run |
| `ScintillatorSD`, `PMTSD` | Sensitive Detektoren |

## Geometrie (Standardwerte)

Die Geometriemaße in der Tabelle sind **Halblängen** (Geant4 `G4Box`). Die über die Kommandozeile übergebene Szintillatorlänge ist dagegen die **volle Länge in y-Richtung** und wird intern halbiert.

| Größe | Wert |
|---|---|
| Szintillator x (Breite) | 7 mm |
| Szintillator y (Länge, Richtung der Photonenkollektoren) | 35 mm |
| Szintillator z (Dicke) | 2 mm |
| World | 100 mm in jede Richtung |

Die Photonenkollektoren sitzen an den beiden y-Enden des Szintillators. Für jede übergebene Länge gilt: Sie muss **größer als 0** und **kleiner als** `DetectorConstruction::kMaxScintLengthY` sein (bisher ca. 199,998 mm einschließlich Kollektoren).

## Voraussetzungen

- Geant4 (mit `ui_all` und `vis_all`, optischer Physik, Analysis/CSV)
- CMake ≥ 3.16
- C++17-Compiler

## Bauen

Geant4-Umgebung laden (Installationspfad anpassen), dann bauen:

```bash
source /pfad/zu/geant4-install/bin/geant4.sh
mkdir -p build && cd build
cmake ..
make -j
```

**Wichtig:** Die erweiterte Hauptdatei muss im Projekt als `Scintillation_Simulation.cc` verwendet und das Projekt neu gebaut werden.

## Ausführen

### Interaktiver Modus (Visualisierung)

```bash
./Scintillation_Simulation
```

Ohne Kommandozeilenargumente wird der interaktive Modus gestartet und `vis.mac` ausgeführt (OpenGL-Viewer, standardmäßig 1 Event).

### Batch-Modus

```bash
./Scintillation_Simulation <macro> [events] [energies] [energieeinheit] [lengths] [materials]
```

Alle Argumente nach `<macro>` sind optional. **Einzelwerte und Listen** werden für `energies`, `lengths` und `materials` akzeptiert.

| Argument | Bedeutung | Standard |
|---|---|---|
| `macro` | Auszuführende Makrodatei, z. B. `run.mac` | Pflicht im Batch-Modus |
| `events` | Anzahl Events **pro Kombination** (positive ganze Zahl) | `1` |
| `energies` | Eine Energie oder Liste, z. B. `1`, `500MeV` oder `"[1MeV,2MeV,1GeV]"` | `1` |
| `energieeinheit` | Standardeinheit für Energieeinträge **ohne** eigene Einheit | `GeV` |
| `lengths` | Volle Szintillatorlängen mit Einheit, z. B. `70mm` oder `"[50mm,70mm,100mm]"` | `"[70mm]"` |
| `materials` | Ein Material oder eine Liste, z. B. `EJ232` oder `"[EJ228,EJ230,EJ232]"` | `EJ232` |

#### Schreibweise der Listen und Einheiten

- Listen werden als **kommagetrennte Werte in eckigen Klammern** angegeben, z. B. `"[EJ228,EJ230]"`. Listenargumente am besten in Anführungszeichen setzen, damit die Shell sie unverändert weitergibt.
- Ein einzelner Wert funktioniert weiterhin **ohne** eckige Klammern, z. B. `1`, `70mm` oder `EJ232`.
- **Energien:** Zulässige Einheiten sind `eV`, `keV`, `MeV`, `GeV` und `TeV`. Die Einheit darf pro Listeneintrag stehen, etwa `"[1MeV,2MeV,1GeV]"`. Bei `"[1,2,3]"` gilt für alle Einträge das Argument `energieeinheit`.
- **Längen:** Jede Länge benötigt ausdrücklich ihre eigene Einheit: `um`, `mm` oder `cm`, z. B. `"[50mm,7cm,100mm]"`. Die Werte müssen positiv sein und in die World-Geometrie passen.
- **Materialien:** Namen sind **groß-/kleinschreibungssensitiv** und müssen in `OpticalMaterials::kScintillatorNames` registriert sein. Beispiele sind `EJ228`, `EJ230`, `EJ232` und `EJ232Q05`, sofern sie in dieser Liste enthalten sind.
- Leere Listen, nicht unterstützte Einheiten und ungültige Materialnamen führen zu einer Fehlermeldung.

### Beispiele

```bash
# Standardwerte: 1 Event, 1 GeV, 70 mm, EJ232
./Scintillation_Simulation run.mac

# Bisherige Syntax mit Einzelwerten bleibt gültig:
# 1000 Events, 500 MeV, 70 mm, EJ232
./Scintillation_Simulation run.mac 1000 500 MeV

# 1000 Events, 1 GeV, 50 mm, EJ230
./Scintillation_Simulation run.mac 1000 1 GeV "[50mm]" EJ230

# Mehrere Längen bei einer Energie und einem Material: 3 Runs
./Scintillation_Simulation run.mac 1000 1 GeV "[100mm,70mm,50mm]" EJ232

# Mehrere Energien ohne Einheit im Array: Standard-Einheit GeV, 3 Runs
./Scintillation_Simulation run.mac 1000 "[1,2,3]" GeV "[70mm]" EJ232

# Mehrere Energien mit eigenen Einheiten: 3 Runs
./Scintillation_Simulation run.mac 1000 "[1MeV,2MeV,1GeV]" MeV "[70mm]" EJ232

# Mehrere Materialien bei einer Energie und Länge: 3 Runs
./Scintillation_Simulation run.mac 1000 1 GeV "[70mm]" "[EJ228,EJ230,EJ232]"

# Alle drei Listen kombinieren: 3 Energien × 3 Längen × 4 Materialien = 36 Runs
./Scintillation_Simulation run.mac 1000 \
    "[1MeV,2MeV,1GeV]" MeV \
    "[50mm,70mm,100mm]" \
    "[EJ228,EJ230,EJ232,EJ232Q05]"
```

Beim letzten Beispiel werden **36 Runs mit jeweils 1.000 Events**, also insgesamt **36.000 Events**, ausgeführt.

### Ablauf bei mehreren Parametern

Die Simulation bildet das **kartesische Produkt** aus allen angegebenen Materialien, Längen und Energien. Die Schleifenreihenfolge ist:

1. **Material** auswählen.
2. **Länge** auswählen und einen neuen `RunManager` mit passender `DetectorConstruction` erstellen.
3. Makro für diese Material-Längen-Kombination ausführen.
4. **Alle Energien** nacheinander über `/gun/energy` einstellen und jeweils `/run/beamOn <events>` ausführen.
5. Den `RunManager` freigeben und zur nächsten Material-Längen-Kombination wechseln.

Die Anzahl der Runs ergibt sich aus:

```text
Runs = Anzahl Materialien × Anzahl Längen × Anzahl Energien
Gesamtzahl Events = Runs × Events pro Run
```

**Hinweis zu `run.mac`:** Das Makro wird **einmal pro Material-Längen-Kombination** ausgeführt, nicht einmal pro Energie. Es sollte den Run initialisieren, bevor `/run/beamOn` aufgerufen wird. Die Energie wird nach dem Makro durch `/gun/energy` gesetzt; eine Energieeinstellung im Makro wird daher für die Batch-Runs überschrieben. Enthält das Makro selbst `/run/beamOn`, löst es zusätzliche Runs aus.

## Ausgabe

- **Konsole:** Pro Event werden unter anderem Primärteilchen, Energiedeposition, Zeit des Elektroneneintritts, Photonenzahlen oben/unten/gesamt, Asymmetrie und Rechenzeit ausgegeben. Für jeden Run erscheint zusätzlich eine Zusammenfassung; bei mehreren Parameterkombinationen wird die aktuelle Simulation mit Material, Länge und Energie angekündigt.
- **CSV-Dateien:** Nach Material und Energie sortiert im Schema `data/<material>/<energie>/<länge>mm.csv`, z. B. `data/EJ230/1GeV/70mm.csv`. Die Ausgabeorganisation entspricht der bisherigen Implementierung in `RunAction`. **Eine erneut ausgeführte identische Kombination kann eine vorhandene CSV-Datei überschreiben.** Pro Photon-Treffer wird ein Datensatz geschrieben.

Beispiel für den letzten Batch-Aufruf:

```text
data/
├── EJ228/
│   ├── 1MeV/
│   │   ├── 50mm.csv
│   │   ├── 70mm.csv
│   │   └── 100mm.csv
│   ├── 2MeV/
│   └── 1GeV/
├── EJ230/
├── EJ232/
└── EJ232Q05/
```

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
| `scintLength` | Volle Länge des Szintillators in mm |
| `emissionAngle` | Winkel zwischen Photonenerzeugung und Strahlrichtung in Grad |
| `material` | Name des Szintillatormaterials, z. B. `EJ230` |
| `electronEnergy` | Energie des Primärelektrons in MeV |

## Anderes Szintillatormaterial verwenden

1. In `include/OpticalMaterials.hh` und `src/OpticalMaterials.cc` eine Materialdefinition ergänzen, z. B. `G4Material* CreateEJ200();`, mit Zusammensetzung, Dichte und `G4MaterialPropertiesTable` (`RINDEX`, `ABSLENGTH`, `SCINTILLATIONCOMPONENT1`, `SCINTILLATIONYIELD`, `RESOLUTIONSCALE`, `SCINTILLATIONTIMECONSTANT1`, ...).
2. Das neue Material in der Materialauswahl der `DetectorConstruction` berücksichtigen und seinen Namen in `OpticalMaterials::kScintillatorNames` aufnehmen. Ein bloßes Ändern eines festen Standardmaterials reicht für die Batch-Materialliste nicht aus.
3. Projekt neu bauen. Anschließend kann das Material als einzelner Name oder in einer Liste übergeben werden.

## Makros

- `run.mac`: Batch-Einstellungen (`/run/initialize`, Verbosity, Teilchen, Energie, Richtung). Bei Listenaufrufen wird die Energie anschließend programmatisch gesetzt.
- `vis.mac`: Visualisierung und Trajektorien; enthält auskommentierte Filter und Einfallswinkel-Beispiele.