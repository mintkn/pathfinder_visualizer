#include "Pathfinder.h"
#include "Utils.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <random>

void PathfinderBase::wait(Grid& grid, TDT4102::AnimationWindow& win)
{
    if(instant) return;

    grid.draw(win);
    draw_panel(win, grid, *this);
    win.next_frame();
    TDT4102::AnimationWindow::wait_for(animation_step);
}

int PathfinderBase::heuristic(int r, int c, int end_r, int end_c) const
{
    return std::abs(r-end_r) + std::abs(c-end_c);
}

void PathfinderBase::animate_path(int nr, int nc, const std::optional<std::pair<int, int>>& start, Grid& grid, TDT4102::AnimationWindow& win)
{
    std::vector<std::vector<int>> path;
    std::pair<int,int> pos = {nr, nc};

    while(pos != *start)
    {
        path.push_back({pos.first, pos.second});
        pos = parent[pos.first][pos.second];
    }
    std::reverse(path.begin(), path.end());
    final_path = path;


    int ROWS = grid.get_rows();
    int COLS = grid.get_cols();

    for(int r = 0; r < ROWS; r++)
    for(int c = 0; c < COLS; c++)
        if(grid.get_cell(r,c) == CellState::Visited)
        grid.set_cell(r,c, CellState::Empty);


    for(auto& p : path)
    {
        if(grid.get_cell(p[0], p[1]) == CellState::End) continue;
        wait(grid, win);
        grid.set_cell(p[0], p[1], CellState::Path);
    }
}

bool PathFinder::find_path(Grid& grid, TDT4102::AnimationWindow& win)
{
    std::optional<std::pair<int,int>> start = grid.find_start();
    std::optional<std::pair<int,int>> end   = grid.find_end();

    if(!start || !end)
        return false;

    int ROWS = grid.get_rows();
    int COLS = grid.get_cols();

    final_path.clear();
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    parent.assign(ROWS, std::vector<std::pair<int,int>>(COLS, {-1, -1}));

    std::queue<std::pair<int,int>> q;
    q.push(*start);
    visited[start->first][start->second] = true;

    while(!q.empty())
    {
        std::pair<int,int> curr = q.front();
        q.pop();

        for(std::pair<int,int> dir : direc)
        {
            int nr = curr.first  + dir.first;
            int nc = curr.second + dir.second;

            if(nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if(visited[nr][nc]) continue;

            CellState check = grid.get_cell(nr, nc);
            if(check == CellState::Wall) continue;

            parent[nr][nc] = curr;

            if(check == CellState::End)
            {
                animate_path(nr, nc, start, grid, win);
                return true;
            }

            visited[nr][nc] = true;
            grid.set_cell(nr, nc, CellState::Visited);
            wait(grid, win);
            q.push({nr, nc});
        }
    }

    return false;
}


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

    std::priority_queue<Node, std::vector<Node>,std::greater<Node>> pq;

    int h0 = heuristic(start->first, start->second, end_r, end_c);
    pq.push({h0, 0, h0, start->first, start->second});
    visited[start->first][start->second] = true;

    while(!pq.empty())
    {
        Node current = pq.top();
        pq.pop();

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
            pq.push({h, 0, h, nr, nc});
        }
    }
    return false;
}

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
            int g = current.path_cost + 1;
            int f = g + h;
            pqa.push({f, g, h, nr, nc});
        }
    }
    return false;
}


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

    std::queue<std::pair<int,int>> q;
    q.push(*start);
    visited[start->first][start->second] = true;

    std::pair<int,int> current = *start;
    const int max_steps = ROWS * COLS * 10;
    int steps = 0;

    while(steps++ < max_steps)
    {
        auto neighbors = free_neighbors(current);

        if(neighbors.empty())
        {
            // Dead end 
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
            if(!found) return false;
            continue;
        }


        std::pair<int,int> next = neighbors[rand() % static_cast<int>(neighbors.size())];
        int nr = next.first;
        int nc = next.second;

        parent[nr][nc] = current;

        if(grid.get_cell(nr, nc) == CellState::End)
        {
            animate_path(nr, nc, start, grid, win);
            return true;
        }

        visited[nr][nc] = true;
        grid.set_cell(nr, nc, CellState::Visited);
        wait(grid, win);
        q.push(next);
        current = next;
    }
    return false;
}

