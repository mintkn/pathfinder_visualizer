# Pathfinding Visualizer

Et interaktivt verktøy for å visualisere og sammenligne pathfinding-algoritmer på et rutenett. Bygget i C++ med TDT4102 AnimationWindow-biblioteket.

## Algoritmer

| Navn | Beskrivelse |
|------|-------------|
| **BFS** | Bredde-først-søk. Garanterer korteste sti. |
| **Greedy Best-First** | Prioriterer celler nærmest målet. Rask, men ikke alltid optimal. |
| **A\*** | Kombinerer faktisk kostnad (g) og heuristikk (h). Optimal og effektiv. |
| **Random** | Tilfeldig gange — velger én tilfeldig retning per steg, faller tilbake til køen ved dead end. |

## Kontroller

| Tast | Funksjon |
|------|----------|
| `ENTER` | Kjør valgt algoritme |
| `B` | Bytt algoritme |
| `R` | Reset grid |
| `BACKSPACE` | Angre siste endring |
| `I` | Veksle instant-modus (hopp over animasjon) |
| `M` / `N` | Raskere / saktere animasjon |
| `S` / `L` | Lagre / last grid fra fil |
| `1–9` | Last inn demo-labyrint |
| `NUM +` / `NUM -` | Øk / minsk gridstørrelse |
| `ESCAPE` | Avslutt |

**Musekontroller:**
- Venstreklikk og dra — tegn/slett vegger
- Høyreklikk — plasser start- og målpunkt (sykler mellom Start og Mål)


## Struktur

- `Grid` — rutenett med celletilstander (Empty, Wall, Start, End, Visited, Path)
- `PathfinderBase` — abstrakt basisklasse for alle algoritmer
- `Utils` — panel-tegning, input-håndtering og animasjonslogikk
- `demo_mazes` — forhåndsdefinerte labyrintkart
