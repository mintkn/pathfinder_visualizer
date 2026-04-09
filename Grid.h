#pragma once

#include "AnimationWindow.h"
#include "Cell.h"
#include "GridException.h"
#include "Utils.h"
#include <vector>
#include <string>
#include <optional>


// Abstrakt basisklasse — alle grid-typer må implementere draw og reset
class GridBase
{
    public:
        virtual void draw(TDT4102::AnimationWindow& win) = 0;
        virtual void reset() = 0;
        virtual ~GridBase() = default;
};


class Grid : public GridBase
{
    private:
        int rows;
        int cols;
        std::vector<std::vector<CellState>> cells;
        int cell_size;
        int current_demo = -1;

        void compute_cell_size();
        TDT4102::Color state_to_color(CellState s) const;

    public:
        Grid(int rows, int cols);

        // Lesbare getters
        int get_rows() const { return rows; }
        int get_cols() const { return cols; }
        int get_current_demo() const { return current_demo; }
        void set_current_demo(int d) { current_demo = d; }

        // Celle-tilgang for MouseHandler
        CellState get_cell(int r, int c) const { return cells[r][c]; }
        void set_cell(int r, int c, CellState s) { cells[r][c] = s; }

        // Grid UI
        void draw(TDT4102::AnimationWindow& win) override;
        void reset() override;

        // Musposisjon → celle
        std::optional<std::pair<int,int>> cell_at_pos(int mx, int my) const;

        void right_click(int r, int c);

        std::optional<std::pair<int, int>> find_start() const;
        std::optional<std::pair<int, int>> find_end() const;
        int count_state(CellState s) const;

        void resize(int delta);

        void save_to_file(const std::string& filename) const;
        void load_from_file(const std::string& filename);
        void load_demo_maze(const Maze& maze);

        void generate_maze();

};

inline const std::vector<std::pair<int, int>> direc_gen = {
    {-2, 0}, {2, 0},
    {0, -2}, {0, 2}
};