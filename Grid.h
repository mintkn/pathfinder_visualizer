#pragma once

#include "AnimationWindow.h"
#include "Cell.h"
#include "GridException.h"
#include "Utils.h"
#include <vector>
#include <string>
#include <optional>


// Bruker pure virtual functions (= 0) slik at man ikke kan lage en GridBase direkte
class GridBase
{
    public:
        virtual void draw(TDT4102::AnimationWindow& win) = 0;
        virtual void reset() = 0;
        virtual ~GridBase() = default; // viktig å ha virtuell destruktør når man bruker arv
};


// Hoved-grid-klassen som holder på alle cellene og tegner dem
// Arver fra GridBase og implementerer draw og reset
class Grid : public GridBase
{
    private:
        int rows;
        int cols;
        // 2D-vektor som lagrer tilstanden til hver celle
        std::vector<std::vector<CellState>> cells;
        int cell_size; // hvor stor hver celle er i piksler
        int current_demo = -1; // hvilken demo-labyrint som er lastet (-1 = ingen)

        // Regner ut cellesstørrelsen basert på vindusstørrelsen og antall rader/kolonner
        void compute_cell_size();
        // Returnerer fargen som hører til en gitt celletilstand
        TDT4102::Color state_to_color(CellState s) const;

    public:
        // Konstruktør - lager et tomt grid med gitt antall rader og kolonner
        Grid(int rows, int cols);

        // Lesbare getters - bruker const fordi de ikke endrer noe
        int get_rows() const { return rows; }
        int get_cols() const { return cols; }
        int get_current_demo() const { return current_demo; }
        void set_current_demo(int d) { current_demo = d; }

        // Celle-tilgang for MouseHandler
        CellState get_cell(int r, int c) const { return cells[r][c]; }
        void set_cell(int r, int c, CellState s) { cells[r][c] = s; }

        // Tegner hele gridet til vinduet
        void draw(TDT4102::AnimationWindow& win) override;
        // Setter alle celler tilbake til Empty
        void reset() override;

        // Konverterer musposisjon (piksler) til rad/kolonne i gridet
        // Returnerer optional fordi musa kan være utenfor gridet
        std::optional<std::pair<int,int>> cell_at_pos(int mx, int my) const;

        // Høyreklikk-logikk: setter start/slutt eller fjerner dem
        void right_click(int r, int c);

        // Søker gjennom alle celler etter start- eller sluttcellen
        std::optional<std::pair<int, int>> find_start() const;
        std::optional<std::pair<int, int>> find_end() const;

        // Teller hvor mange celler som har en bestemt tilstand
        int count_state(CellState s) const;

        // Endrer størrelsen på gridet
        void resize(int delta);

        // Lagrer og laster gridet fra fil
        void save_to_file(const std::string& filename) const;
        void load_from_file(const std::string& filename);

        // Laster inn en ferdiglaget demo-labyrint
        void load_demo_maze(const Maze& maze);

        // Genererer en tilfeldig labyrint med DFS (depth-first search)
        void generate_maze();

};

// Retninger for labyrintgenerering - hopper 2 celler av gangen
// Dette er for at det skal bli vegger mellom cellene
inline const std::vector<std::pair<int, int>> direc_gen = {
    {-2, 0}, {2, 0},
    {0, -2}, {0, 2}
};
