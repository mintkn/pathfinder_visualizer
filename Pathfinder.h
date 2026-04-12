#pragma once

#include "AnimationWindow.h"
#include "Grid.h"
#include <vector>
#include <string>

// Node brukes av Greedy og A* til å lagre kostnad og posisjon for hver celle
// total_cost er det som bestemmer rekkefølgen i priority queue
struct Node
{
    int total_cost;      // f = g + h (brukes til å sortere køen)
    int path_cost;       // g = antall steg fra start
    int heuristic_cost;  // h = estimert avstand til mål
    int row;
    int col;

    // sammenligner noder basert på total_cost - brukes av priority_queue. definerer hva ">" skal bety for Node objekter
    bool operator>(const Node& other) const { return total_cost > other.total_cost; }
};


// Felles data (final_path, parent) ligger her; animation_step og instant er
// globale variabler i Utils.h slik at draw_panel fungerer for alle algoritmer
class PathfinderBase
{
    public:

        // Venter litt mellom hvert steg slik at man kan se animasjonen
        void wait(Grid& grid, TDT4102::AnimationWindow& win);

        // Tegner opp den ferdige stien etter at målet er funnet
        void animate_path(int nr, int nc, const std::optional<std::pair<int, int>>& start, Grid& grid, TDT4102::AnimationWindow& win);

        // Returnerer den ferdige stien (liste med [rad, kolonne] for hver celle)
        const std::vector<std::vector<int>>& get_final_path() const { return final_path; }

        // Alle underklasser må implementere disse to
        virtual bool find_path(Grid& grid, TDT4102::AnimationWindow& win) = 0;
        virtual std::string name() const = 0;
        virtual ~PathfinderBase() = default;
    protected:
        std::vector<std::vector<int>> final_path;  // den ferdige stien fra start til mål
        std::vector<std::vector<std::pair<int, int>>> parent;  // hvem kom vi fra for å nå denne cellen
        // Manhattan-avstand som heuristikk - rett fram er enklest og fungerer bra her
        int heuristic(int r, int c, int end_r, int end_c) const;
};


// BFS - Breadth First Search, garanterer korteste vei
class PathFinder : public PathfinderBase
{
    public:
        bool find_path(Grid& grid, TDT4102::AnimationWindow& win) override;
        std::string name() const override { return "BFS"; }
};


// Greedy Best-First Search - ser bare på heuristikken, ikke total kostnad
// Rask, men finner ikke alltid korteste vei
class GreedyFinder : public PathfinderBase
{
    public:
        bool find_path(Grid& grid, TDT4102::AnimationWindow& win) override;
        std::string name() const override { return "Greedy"; }
};

// A* - kombinerer faktisk kostnad (g) og heuristikk (h)
// Som oftest kortesdte veien
class ASTARFinder : public PathfinderBase
{
    public:
        bool find_path(Grid& grid, TDT4102::AnimationWindow& win) override;
        std::string name() const override { return "A*"; }
};

// RandomFinder - beveger seg tilfeldig rundt i gridet
// Finner alltid frem til slutt (hvis det finnes en vei), men veldig ineffektivt
class RandomFinder : public PathfinderBase
{
    public:
        bool find_path(Grid& grid, TDT4102::AnimationWindow& win) override;
        std::string name() const override { return "Random"; }

};

