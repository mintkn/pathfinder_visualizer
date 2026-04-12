#include "Pathfinder.h"
#include "Utils.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <random>

// Venter litt mellom hvert animasjonssteg
// Hvis instant er aktivert hopper vi bare over ventetiden
void PathfinderBase::wait(Grid& grid, TDT4102::AnimationWindow& win)
{
    if(instant) return; // ikke vent hvis instant-modus er på

    grid.draw(win);
    draw_panel(win, grid, *this);
    win.next_frame();
    TDT4102::AnimationWindow::wait_for(animation_step);
}

// Manhattan-avstand: antall steg horisontalt + vertikalt til målet
// Brukes som heuristikk i Greedy og A*
int PathfinderBase::heuristic(int r, int c, int end_r, int end_c) const
{
    return std::abs(r-end_r) + std::abs(c-end_c);
}

// Rekonstruerer stien fra slutt til start ved hjelp av parent-tabellen
// Tegner deretter stien celle for celle med animasjon
void PathfinderBase::animate_path(int nr, int nc, const std::optional<std::pair<int, int>>& start, Grid& grid, TDT4102::AnimationWindow& win)
{
    std::vector<std::vector<int>> path;
    std::pair<int,int> pos = {nr, nc};

    // følger parent bakover fra mål til start
    while(pos != *start)
    {
        path.push_back({pos.first, pos.second});
        pos = parent[pos.first][pos.second];
    }
    // stien er baklengs nå, så vi snur den
    std::reverse(path.begin(), path.end());
    final_path = path;

    int ROWS = grid.get_rows();
    int COLS = grid.get_cols();

    // fjerner alle besøkte celler slik at bare stien vises
    for(int r = 0; r < ROWS; r++)
    for(int c = 0; c < COLS; c++)
        if(grid.get_cell(r,c) == CellState::Visited)
        grid.set_cell(r,c, CellState::Empty);

    // tegner stien steg for steg
    for(auto& p : path)
    {
        if(grid.get_cell(p[0], p[1]) == CellState::End) continue; // ikke overskriv slutt-cellen
        wait(grid, win);
        grid.set_cell(p[0], p[1], CellState::Path);
    }
}

// BFS - Breadth First Search
// Utforsker alle naboer lag for lag, garanterer korteste vei
bool PathFinder::find_path(Grid& grid, TDT4102::AnimationWindow& win)
{
    std::optional<std::pair<int,int>> start = grid.find_start();
    std::optional<std::pair<int,int>> end   = grid.find_end();

    // returner false hvis start eller slutt mangler
    if(!start || !end)
        return false;

    int ROWS = grid.get_rows();
    int COLS = grid.get_cols();

    final_path.clear();
    // visited holder styr på hvilke celler vi allerede har sett på
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    // parent brukes til å rekonstruere stien etterpå
    parent.assign(ROWS, std::vector<std::pair<int,int>>(COLS, {-1, -1}));

    // BFS bruker en vanlig kø (FIFO)
    std::queue<std::pair<int,int>> q;
    q.push(*start);
    visited[start->first][start->second] = true;

    while(!q.empty())
    {
        std::pair<int,int> curr = q.front();
        q.pop();

        // sjekker alle fire naboer (opp, ned, venstre, høyre)
        for(std::pair<int,int> dir : direc)
        {
            int nr = curr.first  + dir.first;
            int nc = curr.second + dir.second;

            // hopp over hvis utenfor gridet
            if(nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if(visited[nr][nc]) continue;

            CellState check = grid.get_cell(nr, nc);
            if(check == CellState::Wall) continue; // kan ikke gå gjennom vegger

            parent[nr][nc] = curr; // husker hvem vi kom fra

            if(check == CellState::End)
            {
                // fant slutt! Tegn stien og returner true
                animate_path(nr, nc, start, grid, win);
                return true;
            }

            visited[nr][nc] = true;
            grid.set_cell(nr, nc, CellState::Visited);
            wait(grid, win);
            q.push({nr, nc});
        }
    }

    return false; // ingen vei funnet
}


// Greedy Best-First Search
// Velger alltid cellen som ser nærmest ut til målet (bare heuristikk, ikke faktisk kostnad)
bool GreedyFinder::find_path(Grid& grid, TDT4102::AnimationWindow& win)
{
    std::optional<std::pair<int,int>> start = grid.find_start();
    std::optional<std::pair<int,int>> end   = grid.find_end();

    if(!start || !end)
        return false;

    int ROWS = grid.get_rows();
    int COLS = grid.get_cols();

    int end_r = end->first;
    int end_c = end->second;

    final_path.clear();
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    parent.assign(ROWS, std::vector<std::pair<int,int>>(COLS, {-1, -1}));

    // priority_queue med std::greater sorterer slik at laveste kostnad kommer først
    std::priority_queue<Node, std::vector<Node>,std::greater<Node>> pq;

    // starter med heuristikken fra startpunktet
    int h0 = heuristic(start->first, start->second, end_r, end_c);
    pq.push({h0, 0, h0, start->first, start->second});
    visited[start->first][start->second] = true;

    while(!pq.empty())
    {
        Node current = pq.top();
        pq.pop();

        // marker cellen som besøkt hvis den er tom
        if(grid.get_cell(current.row, current.col) == CellState::Empty)
        {
            grid.set_cell(current.row, current.col, CellState::Visited);
            wait(grid, win);
        }

        for(std::pair<int, int> dir : direc)
        {
            int nr = current.row + dir.first;
            int nc = current.col + dir.second;

            if(nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if(visited[nr][nc]) continue;

            CellState check = grid.get_cell(nr, nc);
            if(check == CellState::Wall) continue;

            parent[nr][nc] = {current.row, current.col};
            if(check == CellState::End)
            {
                animate_path(nr, nc, start, grid, win);
                return true;
            }

            visited[nr][nc] = true;
            // Greedy bruker bare heuristikken (h), ikke g-kostnaden
            int h = heuristic(nr, nc, end_r, end_c);
            pq.push({h, 0, h, nr, nc});
        }
    }
    return false;
}

// A* (A-star) - kombinerer faktisk kostnad (g) og heuristikk (h) til f = g + h
// Garanterer korteste vei og er vanligvis raskere enn BFS på store grids
bool ASTARFinder::find_path(Grid& grid, TDT4102::AnimationWindow& win)
{
    std::optional<std::pair<int, int>> start = grid.find_start();
    std::optional<std::pair<int, int>> end = grid.find_end();

    if(!end || !start)
        return false;

    int ROWS = grid.get_rows();
    int COLS = grid.get_cols();

    int end_r = end->first;
    int end_c = end->second;

    int start_r = start->first;
    int start_c = start->second;

    final_path.clear();
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    parent.assign(ROWS, std::vector<std::pair<int,int>>(COLS, {-1, -1}));

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pqa;

    // startknoden har g=0 og f = h (ingen steg gjort ennå)
    int h0 = heuristic(start_r, start_c, end_r, end_c);
    int g0 = 0;
    int f0 = h0 + g0;
    pqa.push({f0, g0, h0, start_r, start_c});
    visited[start_r][start_c] = true;

    while(!pqa.empty())
    {
        Node current = pqa.top();
        pqa.pop();

        if(grid.get_cell(current.row, current.col) == CellState::Empty)
        {
            grid.set_cell(current.row, current.col, CellState::Visited);
            wait(grid, win);
        }

        for(std::pair<int, int> dir : direc)
        {
            int nr = current.row + dir.first;
            int nc = current.col + dir.second;

            if(nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if(visited[nr][nc]) continue;

            CellState check = grid.get_cell(nr, nc);
            if(check == CellState::Wall) continue;

            parent[nr][nc] = {current.row, current.col};
            if(check == CellState::End)
            {
                animate_path(nr, nc, start, grid, win);
                return true;
            }

            visited[nr][nc] = true;
            int h = heuristic(nr, nc, end_r, end_c);
            int g = current.path_cost + 1; // ett steg til
            int f = g + h;                 // total estimert kostnad
            pqa.push({f, g, h, nr, nc});
        }
    }
    return false;
}


// RandomFinder - vandrer tilfeldig rundt i gridet inntil den finner veien
// Ikke effektiv, men morsom å se på - kan ta veldig mange steg
bool RandomFinder::find_path(Grid& grid, TDT4102::AnimationWindow& win)
{
    std::optional<std::pair<int, int>> start = grid.find_start();
    std::optional<std::pair<int, int>> end = grid.find_end();

    if(!end || !start)
        return false;

    int ROWS = grid.get_rows();
    int COLS = grid.get_cols();

    final_path.clear();
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    parent.assign(ROWS, std::vector<std::pair<int,int>>(COLS, {-1, -1}));

    // lambda som returnerer alle ubesøkte naboer vi kan gå til
    //låner alle variabler rundt seg ved referanse
    auto free_neighbors = [&](std::pair<int,int> pos)
    {
        std::vector<std::pair<int,int>> result;
        for(std::pair<int, int> dir : direc)
        {
            int nr = pos.first + dir.first;
            int nc = pos.second + dir.second;
            if(nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if(visited[nr][nc]) continue;
            if(grid.get_cell(nr, nc) == CellState::Wall) continue;
            result.push_back({nr, nc});
        }
        return result;
    };

    // bruker en kø for å huske alle celler vi har vært innom (for backtracking)
    std::queue<std::pair<int,int>> q;
    q.push(*start);
    visited[start->first][start->second] = true;

    std::pair<int,int> current = *start;
    // sett en grense på antall steg så vi ikke looper evig
    const int max_steps = ROWS * COLS * 10;
    int steps = 0;

    while(steps++ < max_steps)
    {
        auto neighbors = free_neighbors(current);

        if(neighbors.empty())
        {
            // dead end - ingen ubesøkte naboer, backtrack til en celle med muligheter
            bool found = false;
            while(!q.empty())
            {
                current = q.front();
                q.pop();
                if(!free_neighbors(current).empty())
                {
                    q.push(current);
                    found = true;
                    break;
                }
            }
            if(!found) return false; // gridet er helt utforsket uten å finne slutt
            continue;
        }

        // velger en tilfeldig nabo og beveger seg dit
        std::pair<int,int> next = neighbors[rand() % static_cast<int>(neighbors.size())];
        int nr = next.first;
        int nc = next.second;

        parent[nr][nc] = current; // husk hvem vi kom fra

        if(grid.get_cell(nr, nc) == CellState::End)
        {
            // fant slutten!
            animate_path(nr, nc, start, grid, win);
            return true;
        }

        visited[nr][nc] = true;
        grid.set_cell(nr, nc, CellState::Visited);
        wait(grid, win);
        q.push(next);
        current = next;
    }
    return false; // nådde maks antall steg uten å finne veien
}

