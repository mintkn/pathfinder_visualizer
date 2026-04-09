#include "Grid.h"
#include <fstream>
#include <algorithm>
#include "Utils.h"
#include <stack>



Grid::Grid(int rows, int cols)
    : rows(rows), cols(cols), cells(rows, std::vector<CellState>(cols, CellState::Empty))
{
    compute_cell_size();
}

void Grid::compute_cell_size()
{
    int cell_width  = (PANEL_X_START - DEADSPACE) / cols;
    int cell_height = (WIN_HEIGHT - 2 * DEADSPACE) / rows;
    cell_size = std::min(cell_width, cell_height);
}

void Grid::draw(TDT4102::AnimationWindow& win)
{
    win.setBackgroundColor(COLOR_BG);
    for(int row = 0; row < rows; row++)
        for(int col = 0; col < cols; col++) {
            int x = col * cell_size + DEADSPACE;
            int y = row * cell_size + DEADSPACE;
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

void Grid::reset()
{
    for(int row = 0; row < rows; row++)
        for(int col = 0; col < cols; col++)
            cells[row][col] = CellState::Empty;
}

std::optional<std::pair<int,int>> Grid::cell_at_pos(int mx, int my) const
{
    int c = (mx - DEADSPACE) / cell_size;
    int r = (my - DEADSPACE) / cell_size;
    if(mx < DEADSPACE || my < DEADSPACE || c >= cols || r >= rows || c < 0 || r < 0)
        return std::nullopt;
    return {{r, c}};
}

void Grid::right_click(int r, int c)
{
    if(cells[r][c] == CellState::Start || cells[r][c] == CellState::End)
    {
        cells[r][c] = CellState::Empty;
    }
    else
    {
        if(count_state(CellState::Start) == 0)
            cells[r][c] = CellState::Start;
        else if(count_state(CellState::End) == 0)
            cells[r][c] = CellState::End;
    }
}

std::optional<std::pair<int, int>> Grid::find_start() const
{
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < cols; c++)
            if(cells[r][c] == CellState::Start) return {{r, c}};
    return std::nullopt;
}

std::optional<std::pair<int, int>> Grid::find_end() const
{
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < cols; c++)
            if(cells[r][c] == CellState::End) return {{r, c}};
    return std::nullopt;
}

int Grid::count_state(CellState s) const
{
    int count = 0;
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < cols; c++)
            if(cells[r][c] == s) count++;
    return count;
}

void Grid::resize(int delta)
{
    int new_size;
    if(rows + delta < GRID_MIN)      new_size = GRID_MIN;
    else if(rows + delta > GRID_MAX) new_size = GRID_MAX;
    else                             new_size = rows + delta;
    rows = new_size;
    cols = new_size;
    cells.assign(rows, std::vector<CellState>(cols, CellState::Empty));
    compute_cell_size();
}

void Grid::save_to_file(const std::string& filename) const
{
    std::ofstream f(filename);

    if(!f)
        throw GridException("Kan ikke åpne filen for skrivning" + filename);

    f << rows << " " << cols << '\n';
    for(int r = 0; r < rows; r++)
    {
        for(int c = 0; c < cols; c++)
        {
            f << static_cast<int>(cells[r][c]);
            if(c+1<cols) f << " ";
        }
        f << '\n';
    }
    f << '\n';

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

void Grid::load_from_file(const std::string& filename)
{
    std::ifstream f(filename);

    if(!f)
        throw GridException("Kan ikke åpne filen: " + filename);

    int load_rows = 0;
    int load_cols = 0;

    if(!(f >> load_rows >> load_cols))
        throw GridException("Ugyldig filformat (mangler dimensjoner): " + filename);

    if(load_rows > GRID_MAX || load_rows < GRID_MIN || load_cols > GRID_MAX || load_cols < GRID_MIN)
        throw GridException(
            "Filen inneholder dimensjoner utenfor tillatt område: " +
            std::to_string(load_rows) + "x" + std::to_string(load_cols)
        );

    rows = load_rows;
    cols = load_cols;
    cells.assign(rows, std::vector<CellState>(cols));
    compute_cell_size();

    const int max_state = static_cast<int>(CellState::Path);

    for(int r = 0; r < rows; r++){
        for(int c = 0; c < cols; c++){
            int v = 0;
            if(!(f >> v))
                throw std::runtime_error("Ikke et fullstendig grid: " + filename);
            if(v < 0 || v > max_state)
                throw GridException("Ugyldig celletilstand: " + std::to_string(v));
            cells[r][c] = static_cast<CellState>(v);
        }
    }
}

void Grid::load_demo_maze(const Maze& maze)
{
    rows = static_cast<int>(maze.size());
    cols = static_cast<int>(maze[0].size());

    cells.assign(rows, std::vector<CellState>(cols, CellState::Empty));
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < cols; c++)
            cells[r][c] = char_to_state(maze[r][c]);

    compute_cell_size();
}

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
    return COLOR_EMPTY;
}

void Grid::generate_maze()
{
    std::optional<std::pair<int, int>> start = find_start();
    std::optional<std::pair<int, int>> end = find_end();

    if(!start || !end)
        throw std::runtime_error("Må velge start punkt og endepunkt");


    if((start->first % 2) != (end->first % 2) || (start->second % 2) != (end->second % 2))
        throw std::runtime_error("Start og End må begge ligge på oddetall- eller partallsposisjoner (samme paritet)");

    
        for(int r = 0; r < rows; r++)
    {
        for(int c = 0; c < cols; c++)
        {
            if(cells[r][c] == CellState::End || cells[r][c] == CellState::Start)
                continue;
            cells[r][c] = CellState::Wall;
        }
    }

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));

    auto free_neighbors = [&](std::pair<int, int> pos)
    {
        std::vector<std::pair<int, int>> result;
        for(const std::pair<int, int> dir : direc_gen)
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

    std::stack<std::pair<int, int>> stack;
    stack.push(*start);
    visited[start->first][start->second] = true;

    while(!stack.empty())
    {

        std::pair<int, int> current = stack.top();
        auto neighbors = free_neighbors(current);

        if(!neighbors.empty())
        {        
            std::pair<int,int> next = neighbors[rand() % neighbors.size()];

            std::pair<int,int> between = {
                (current.first + next.first)/  2,
                (current.second + next.second) / 2
            };

            if(cells[between.first][between.second] != CellState::End && cells[between.first][between.second] != CellState::Start)
                cells[between.first][between.second] = CellState::Empty;
            if(cells[next.first][next.second] != CellState::End && cells[next.first][next.second] != CellState::Start)
                cells[next.first][next.second] = CellState::Empty;
            
            visited[next.first][next.second] = true;

            stack.push(next);
            
        }
        else
        {
            stack.pop();
        }
        
        
    }

}
