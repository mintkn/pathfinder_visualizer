#include "Grid.h"
#include <fstream>
#include <algorithm>
#include "Utils.h"
#include <stack>



// Konstruktør - setter opp gridet med tomt innhold og beregner cellestørrelse
Grid::Grid(int rows, int cols)
    : rows(rows), cols(cols), cells(rows, std::vector<CellState>(cols, CellState::Empty))
{
    compute_cell_size();
}

// Regner ut hvor stor hver celle skal være i piksler
// Tar hensyn til begge dimensjoner og bruker den minste slik at alt får plass
void Grid::compute_cell_size()
{
    int cell_width  = (PANEL_X_START - DEADSPACE) / cols;
    int cell_height = (WIN_HEIGHT - 2 * DEADSPACE) / rows;
    cell_size = std::min(cell_width, cell_height); // bruker min slik at cellen passer i begge retninger
}

// Tegner hele gridet rad for rad, kolonne for kolonne
void Grid::draw(TDT4102::AnimationWindow& win)
{
    win.setBackgroundColor(COLOR_BG);
    for(int row = 0; row < rows; row++)
        for(int col = 0; col < cols; col++) {
            // beregner pikselposisjon for denne cellen
            int x = col * cell_size + DEADSPACE;
            int y = row * cell_size + DEADSPACE;
            // litt mellomrom mellom cellene så det ser penere ut
            int space = (cell_size < 6) ? 1 : 2;

            win.draw_rectangle(
                {x + space, y + space},
                cell_size - 2 * space,
                cell_size - 2 * space,
                state_to_color(cells[row][col]),
                COLOR_GRID_LINE
            );
        }
}

// Nullstiller alle celler til Empty
void Grid::reset()
{
    for(int row = 0; row < rows; row++)
        for(int col = 0; col < cols; col++)
            cells[row][col] = CellState::Empty;
}

// Konverterer muskoordinater (piksler) til rad og kolonne i gridet
// Returnerer nullopt hvis musa er utenfor gridet
std::optional<std::pair<int,int>> Grid::cell_at_pos(int mx, int my) const
{
    int c = (mx - DEADSPACE) / cell_size;
    int r = (my - DEADSPACE) / cell_size;
    // sjekker at vi er innenfor gridets grenser
    if(mx < DEADSPACE || my < DEADSPACE || c >= cols || r >= rows || c < 0 || r < 0)
        return std::nullopt;
    return {{r, c}};
}

// Håndterer høyreklikk på en celle
// Hvis man klikker på start/slutt fjernes den, ellers plasseres start eller slutt
void Grid::right_click(int r, int c)
{
    if(cells[r][c] == CellState::Start || cells[r][c] == CellState::End)
    {
        // fjern start/slutt-markering
        cells[r][c] = CellState::Empty;
    }
    else
    {
        // sett start først, deretter slutt (bare én av hver)
        if(count_state(CellState::Start) == 0)
            cells[r][c] = CellState::Start;
        else if(count_state(CellState::End) == 0)
            cells[r][c] = CellState::End;
    }
}

// Søker gjennom gridet og returnerer posisjonen til startcellen
std::optional<std::pair<int, int>> Grid::find_start() const
{
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < cols; c++)
            if(cells[r][c] == CellState::Start) return {{r, c}};
    return std::nullopt; // fant ingen start
}

// Søker gjennom gridet og returnerer posisjonen til slutt-cellen
std::optional<std::pair<int, int>> Grid::find_end() const
{
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < cols; c++)
            if(cells[r][c] == CellState::End) return {{r, c}};
    return std::nullopt; // fant ingen slutt
}

// Teller antall celler med en bestemt tilstand
// Brukes bl.a. til å sjekke om start/slutt allerede er plassert
int Grid::count_state(CellState s) const
{
    int count = 0;
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < cols; c++)
            if(cells[r][c] == s) count++;
    return count;
}

// Endrer størrelsen på gridet med et gitt steg
// Sørger for at størrelsen holder seg innenfor min og maks
void Grid::resize(int delta)
{
    int new_size;
    if(rows + delta < GRID_MIN)      new_size = GRID_MIN;
    else if(rows + delta > GRID_MAX) new_size = GRID_MAX;
    else                             new_size = rows + delta;
    rows = new_size;
    cols = new_size;
    // lager et helt nytt tomt grid med den nye størrelsen
    cells.assign(rows, std::vector<CellState>(cols, CellState::Empty));
    compute_cell_size();
}

// Lagrer gridet til en tekstfil
// Skriver dimensjonene først, deretter tilstanden til hver celle som tall
void Grid::save_to_file(const std::string& filename) const
{
    std::ofstream f(filename);

    if(!f)
        throw GridException("Kan ikke åpne filen for skrivning" + filename);

    // første linje: antall rader og kolonner
    f << rows << " " << cols << '\n';
    for(int r = 0; r < rows; r++)
    {
        for(int c = 0; c < cols; c++)
        {
            // lagrer celletilstanden som et heltall
            f << static_cast<int>(cells[r][c]);
            if(c+1<cols) f << " ";
        }
        f << '\n';
    }
    f << '\n';

    // skriver også en lesbar versjon med bokstaver (nyttig for å lage demo-mazes manuelt)
    for(int r = 0; r < rows; r++)
    {
        f << "{";
        for(int c = 0; c < cols; c++)
        {
            f << "'" << state_to_char(cells[r][c]) << "'";
            if(c+1<cols) f << ", ";
        }

        if(r+1<rows) f << "},"<<'\n';
        else f << "}";
    }


}

// Laster inn et grid fra fil
// Sjekker at filen er gyldig og at dimensjonene er innenfor tillatt område
void Grid::load_from_file(const std::string& filename)
{
    std::ifstream f(filename);

    if(!f)
        throw GridException("Kan ikke åpne filen: " + filename);

    int load_rows = 0;
    int load_cols = 0;

    // leser dimensjonene fra første linje
    if(!(f >> load_rows >> load_cols))
        throw GridException("Ugyldig filformat (mangler dimensjoner): " + filename);

    // sjekker at gridet ikke er for stort eller for lite
    if(load_rows > GRID_MAX || load_rows < GRID_MIN || load_cols > GRID_MAX || load_cols < GRID_MIN)
        throw GridException(
            "Filen inneholder dimensjoner utenfor tillatt område: " +
            std::to_string(load_rows) + "x" + std::to_string(load_cols)
        );

    rows = load_rows;
    cols = load_cols;
    cells.assign(rows, std::vector<CellState>(cols));
    compute_cell_size();

    // bruker høyeste gyldige verdi for å validere innleste tall
    const int max_state = static_cast<int>(CellState::Path);

    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            int v = 0;
            if(!(f >> v))
                throw std::runtime_error("Ikke et fullstendig grid: " + filename);
            // sjekker at verdien er gyldig
            if(v < 0 || v > max_state)
                throw GridException("Ugyldig celletilstand: " + std::to_string(v));
            cells[r][c] = static_cast<CellState>(v);
        }
    }
}

// Laster inn en hardkodet demo-labyrint fra en vektor med bokstaver
void Grid::load_demo_maze(const Maze& maze)
{
    rows = static_cast<int>(maze.size());
    cols = static_cast<int>(maze[0].size());

    cells.assign(rows, std::vector<CellState>(cols, CellState::Empty));
    // konverterer hver bokstav til riktig CellState ved hjelp av char_to_state
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < cols; c++)
            cells[r][c] = char_to_state(maze[r][c]);

    compute_cell_size();
}

// Returnerer fargen som tilhører en gitt celletilstand
TDT4102::Color Grid::state_to_color(CellState s) const
{
    switch(s)
    {
        case CellState::Empty:   return COLOR_EMPTY;
        case CellState::Wall:    return COLOR_WALL;
        case CellState::Start:   return COLOR_START;
        case CellState::End:     return COLOR_END;
        case CellState::Visited: return COLOR_VISITED;
        case CellState::Path:    return COLOR_PATH;
    }
    return COLOR_EMPTY; // burde ikke skje, men kompilatoren krever en return
}

// Genererer en tilfeldig labyrint med iterativ DFS (recursive backtracking)
// Algoritmen starter fra start-cellen og graver seg gjennom vegger
void Grid::generate_maze()
{
    std::optional<std::pair<int, int>> start = find_start();
    std::optional<std::pair<int, int>> end = find_end();

    // kan ikke generere uten at start og slutt er satt først
    if(!start || !end)
        throw std::runtime_error("Må velge start punkt og endepunkt");

    // fyller hele gridet med vegger
    for(int r = 0; r < rows; r++)
    {
        for(int c = 0; c < cols; c++)
        {
            // passer på å ikke overskrive start og slutt
            if(cells[r][c] == CellState::End || cells[r][c] == CellState::Start)
                continue;
            cells[r][c] = CellState::Wall;
        }
    }

    // åpner naboene rundt slutt-cellen slik at algoritmen kan nå den
    int nr = end->first;
    int nc = end->second;

    for(std::pair<int, int> dir : direc)
    {
        int next_r = nr + dir.first;
        int next_c = nc + dir.second;

        if(next_r < 0 || next_r >= rows || next_c < 0 || next_c >= cols) continue;

        if(cells[next_r][next_c] == CellState::Wall) cells[next_r][next_c] = CellState::Empty;
    }

    // holder styr på hvilke celler som er besøkt
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));

    // lambda-funksjon som returnerer ubesøkte naboer 2 steg unna
    // hopper over én celle slik at det blir vegger mellom gangene
    auto free_neighbors = [&](std::pair<int, int> pos)
    {
        std::vector<std::pair<int, int>> result;
        for(const auto& dir : direc_gen)
        {
            int nr = pos.first + dir.first;
            int nc = pos.second + dir.second;
            if(nr < 0 || nr >= rows || nc < 0 || nc >= cols)
                continue;
            if(visited[nr][nc])
                continue;
            result.push_back({nr, nc});
        }
        return result;
    };

    // bruker en stack for å simulere rekursjon (iterativ DFS)
    std::stack<std::pair<int, int>> stack;
    stack.push(*start);
    visited[start->first][start->second] = true;

    while(!stack.empty())
    {
        std::pair<int, int> current = stack.top();
        auto neighbors = free_neighbors(current);

        if(!neighbors.empty())
        {
            // velger en tilfeldig ubesøkt nabo
            std::pair<int,int> next = neighbors[rand() % neighbors.size()];

            // cellen mellom current og next - den må også åpnes
            std::pair<int,int> between = {
                (current.first + next.first)/  2,
                (current.second + next.second) / 2
            };

            // åpner cellen mellom og neste celle (men ikke start/slutt)
            if(cells[between.first][between.second] != CellState::End && cells[between.first][between.second] != CellState::Start)
                cells[between.first][between.second] = CellState::Empty;
            if(cells[next.first][next.second] != CellState::End && cells[next.first][next.second] != CellState::Start)
                cells[next.first][next.second] = CellState::Empty;

            visited[next.first][next.second] = true;

            // fortsetter fra neste celle
            stack.push(next);

        }
        else
        {
            // ingen ubesøkte naboer - backtrack til forrige celle
            stack.pop();
        }


    }

}
