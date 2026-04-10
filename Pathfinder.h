#pragma once

#include "AnimationWindow.h"
#include "Grid.h"
#include <vector>
#include <string>

struct Node
{
    int total_cost;
    int path_cost;
    int heuristic_cost;
    int row;
    int col;

    bool operator>(const Node& other) const { return total_cost > other.total_cost; }
};

// Abstrakt basisklasse for søkealgoritmer
// Felles data (final_path, parent) ligger her; animation_step og instant er
// globale variabler i Utils.h slik at draw_panel fungerer for alle algoritmer
class PathfinderBase
{
    public:

        void wait(Grid& grid, TDT4102::AnimationWindow& win);
        void animate_path(int nr, int nc, const std::optional<std::pair<int, int>>& start, Grid& grid, TDT4102::AnimationWindow& win);

        const std::vector<std::vector<int>>& get_final_path() const { return final_path; }

        virtual bool find_path(Grid& grid, TDT4102::AnimationWindow& win) = 0;
        virtual std::string name() const = 0;
        virtual ~PathfinderBase() = default;
    protected:
        std::vector<std::vector<int>> final_path;
        std::vector<std::vector<std::pair<int, int>>> parent;
        int heuristic(int r, int c, int end_r, int end_c) const;
};


class PathFinder : public PathfinderBase
{
    public:
        bool find_path(Grid& grid, TDT4102::AnimationWindow& win) override;
        std::string name() const override { return "BFS"; }
};

inline const std::vector<std::pair<int, int>> direc = {
    {-1, 0}, {1, 0},
    {0, -1}, {0, 1}
};

class GreedyFinder : public PathfinderBase
{
    public:
        bool find_path(Grid& grid, TDT4102::AnimationWindow& win) override;
        std::string name() const override { return "Greedy Best-First"; }
};

class ASTARFinder : public PathfinderBase
{
    public:
        bool find_path(Grid& grid, TDT4102::AnimationWindow& win) override;
        std::string name() const override { return "A*"; }
};

class RandomFinder : public PathfinderBase
{
    public:
        bool find_path(Grid& grid, TDT4102::AnimationWindow& win) override;
        std::string name() const override { return "Random"; }

};

