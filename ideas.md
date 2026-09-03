8. Creation Position wäre sehr wertvoll

Für Photonentransport würde ich zusätzlich speichern:

const auto vertexPosition =
    track->GetVertexPosition();

also:

const G4double creationX =
    vertexPosition.x();

const G4double creationY =
    vertexPosition.y();

const G4double creationZ =
    vertexPosition.z();

Dann kannst du später untersuchen:

$$ t_\mathrm{flight}(y_\mathrm{creation}) $$

oder

$$ P(\mathrm{top}\mid y_\mathrm{creation}) $$

und sehen, wie der Erzeugungsort die Detektion beeinflusst.

Das ist für einen langen Scintillator sehr interessant.

9. Photonrichtung mit speichern

Ebenso:

const auto direction =
    track->GetMomentumDirection();

mit:

direction.x()
direction.y()
direction.z()

Dann kannst du untersuchen, welche Startwinkel besonders oft zu top/bottom gelangen und welchen Einfluss Reflektoren haben.