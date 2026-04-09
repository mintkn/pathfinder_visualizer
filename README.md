# Pathfinding Visualizer

Et interaktivt verktøy for å visualisere og sammenligne pathfinding-algoritmer på et rutenett. Bygget i C++ med TDT4102 AnimationWindow-biblioteket.

## Kom i gang

### 1. Installer nødvendige verktøy

- **C++-kompilator med C++23-støtte**, f.eks. GCC via [MSYS2/MinGW](https://www.msys2.org/) på Windows
- **Meson** byggesystem:
  ```
  pip install meson
  ```
- **Ninja** (følger ofte med Meson, ellers):
  ```
  pip install ninja
  ```

### 2. Last ned prosjektet

Klon repoet fra GitHub (den grønne "Code"-knappen → kopier URL):
```
git clone https://github.com/dittbrukernavn/reponavn.git
```
Åpne den nedlastede mappen i VSCode.

### 3. Bygg og kjør

Åpne terminalen i VSCode (Terminal → New Terminal) og kjør:
```
meson setup build
meson compile -C build
./build/program
```

Alle avhengigheter (SDL2, AnimationWindow osv.) lastes ned og bygges automatisk av Meson — du trenger ikke installere noe mer.

---

## Algoritmer

Velg algoritme fra nedtrekksmenyen øverst i panelet.

| Algoritme | Beskrivelse | Finner korteste sti? |
|-----------|-------------|----------------------|
| **BFS** | Bredde-først-søk. Utforsker lag for lag utover fra start. | Ja, alltid |
| **Greedy** | Prioriterer alltid cellen nærmest målet (Manhattan-avstand). Rask, men kan ta omveier. | Nei |
| **A\*** | Kombinerer faktisk avstand fra start (g) og estimert avstand til mål (h). Optimal og effektiv. | Ja, alltid |
| **Random** | Velger tilfeldig nabocel per steg. Ved dead end faller den tilbake til køen og prøver derfra. | Finner alltid en sti hvis det finnes en, men stien er ikke optimal |

---

## Grensesnitt

### Nedtrekksmenyer
- **Algoritme** — velg hvilken pathfinding-algoritme som skal kjøres
- **Demo Maze** — last inn en forhåndsdefinert labyrint (se merknad nedenfor)

### Knapper
| Knapp | Funksjon |
|-------|----------|
| **RUN** | Kjør valgt algoritme på gjeldende rutenett |
| **GENERATE** | Generer tilfeldig labyrint med rekursiv backtracking (se merknad nedenfor) |
| **UNDO** | Angre siste endring |
| **RESET** | Tøm hele rutenettet |
| **SAVE** | Lagre gjeldende rutenett til `grid_save.txt` |
| **LOAD** | Last inn rutenett fra `grid_save.txt` |
| **GRID -** | Gjør rutenettet mindre (steg på 5, minimum 5x5) |
| **GRID +** | Gjør rutenettet større (steg på 5, maksimum 100x100) |

### Slider — animasjonshastighet
Juster hvor lang pause det er mellom hvert steg i animasjonen (0–100 ms). Vises i infopanelet som "Anim. step".

### Checkbox — Instant
Når krysset av hoppes animasjonen over helt og resultatet vises med en gang. Animasjonsslideren er da nedtonet.

---

## Tastaturkontroller

| Tast | Funksjon |
|------|----------|
| `ENTER` | Kjør valgt algoritme |
| `R` | Reset rutenett (tilsvarer RESET-knappen) |
| `BACKSPACE` | Angre siste endring (tilsvarer UNDO) |
| `NUMPAD +` | Øk gridstørrelse |
| `NUMPAD -` | Minsk gridstørrelse |
| `ESCAPE` | Avslutt programmet |

---

## Musekontroller

| Handling | Funksjon |
|----------|----------|
| **Venstreklikk og dra** på tom celle | Tegner vegger |
| **Venstreklikk og dra** på vegg | Sletter vegger |
| **Høyreklikk** på tom celle | Plasserer start (grønn), deretter mål (rød) |
| **Høyreklikk** på start eller mål | Fjerner den |

Rekkefølge ved høyreklikk: første klikk på tom celle setter **start**, andre setter **mål**. Fjern en av dem ved å høyreklikke på den, så kan du plassere på nytt.

---

## Demo Mazes

Nedtrekksmenyen tilbyr 5 demo-labyrintkart. **Demo Maze 5 er ikke implementert** — velger du den vises en feilmelding og rutenettet forblir uendret. De andre (1–4) laster inn forhåndsdefinerte labyrintkart med start og mål allerede plassert.

---

## Generering av labyrint — hvorfor virker det ikke alltid?

GENERATE bruker **rekursiv backtracking**, en algoritme som beveger seg i steg på 2 celler om gangen. Derfor har den to krav:

1. **Både start og mål må være plassert** før du trykker GENERATE.
2. **Start og mål må ha samme paritet** — det vil si at rad- og kolonneposisjonen til begge punktene må enten begge være partall eller begge oddetall.

Eksempel: start på (1,1) og mål på (3,3) fungerer ✓  
Eksempel: start på (1,1) og mål på (2,2) fungerer ikke ✗

Hvis kravene ikke er oppfylt vises en feilmelding og ingen labyrint genereres. Flytt start og mål og prøv igjen.

---

## Infopanel

Øverst i høyre panel vises løpende:
- Valgt algoritme
- Aktiv demo-labyrint (eller "ingen")
- Gjeldende gridstørrelse
- Animasjonshastighet
- Om instant-modus er på eller av

Til høyre vises **stielengder** for hver algoritme etter at de er kjørt, slik at man enkelt kan sammenligne resultatene.
