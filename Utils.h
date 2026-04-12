#pragma once

#include <optional>
#include <map>
#include <vector>
#include <iostream>
#include <string>

#include "AnimationWindow.h"
#include "KeyboardKey.h"
#include "Color.h"
#include "Cell.h"

class Grid;
class PathFinder;
class PathfinderBase;

// Alle handlinger brukeren kan utføre via knapper eller tastatursnarveier
enum class Action { Reset, Generate, Undo, Save, Load, GridMinus, GridPlus };

// Utfører en handling
void execute_action(Action a, Grid& grid, Grid& backup, TDT4102::AnimationWindow& win);

// VINDU - konstanter for vindusstørrelse og posisjon
inline constexpr int WIN_WIDTH = 1500;
inline constexpr int WIN_HEIGHT = 810;
inline constexpr int WIN_X = 50;
inline constexpr int WIN_Y = 50;
inline constexpr int DEADSPACE = 20;           // kant-margin rundt gridet
inline constexpr int PANEL_X_START = 1000;     // x-koordinat der sidepanelet starter
inline constexpr int PANEL_WIDTH = WIN_WIDTH - PANEL_X_START - DEADSPACE; // 480
inline constexpr int PANEL_INFO_HEIGHT = 195;  // høyde på info-boksen øverst i panelet
inline constexpr int WIDGET_Y_START = DEADSPACE + PANEL_INFO_HEIGHT + 20; // Dropdown starter her
inline constexpr int BTN_Y_START = WIDGET_Y_START + 250; // knapper starter her

// GRID - grenser for hvor stort gridet kan være
inline constexpr int GRID_MIN  = 5;
inline constexpr int GRID_MAX  = 70;
inline constexpr int GRID_STEP = 5; // endres med dette steget ved GRID+/GRID-

// Globale variabler som deles mellom klasser
// Bruker extern her og definerer dem i Utils.cpp
extern double animation_step;
extern bool instant;
extern std::vector<std::string> g_algo_names;
extern std::vector<int> g_path_lengths;

// PANEL LAYOUT - layout-konstanter for sidepanelet
inline constexpr int  FONT_SIZE = 16;
inline constexpr int  NEW_LINE = 22;               // linjehopp i piksler
inline constexpr int  PANEL_DEADSPACE_SMALL = 4;
inline constexpr int  PANEL_DEADSPACE_LARGE = 10;
inline constexpr int  WIDGET_WIDTH = PANEL_WIDTH - 2 * PANEL_DEADSPACE_LARGE; // widget-bredde (460)


// LAGRING - filnavnet som brukes ved lagring/lasting
constexpr std::string SAVE_FILE = "grid_save.txt";

// FARGER - RGB-farger for de ulike celletilstandene
inline const TDT4102::Color COLOR_EMPTY   = TDT4102::Color(245, 245, 245); // nesten hvit
inline const TDT4102::Color COLOR_WALL    = TDT4102::Color(245, 230, 110); // gul
inline const TDT4102::Color COLOR_START   = TDT4102::Color(50, 255, 20);   // grønn
inline const TDT4102::Color COLOR_END     = TDT4102::Color(255, 50, 50);   // rød
inline const TDT4102::Color COLOR_VISITED = TDT4102::Color(75, 225, 255);  // lyseblå
inline const TDT4102::Color COLOR_PATH    = TDT4102::Color(200, 100, 255); // lilla
inline const TDT4102::Color COLOR_BG      = TDT4102::Color(160, 160, 160); // grå bakgrunn
inline const TDT4102::Color COLOR_GRID_LINE = TDT4102::Color(130,130, 130);
inline const TDT4102::Color COLOR_TXT     = TDT4102::Color(255, 255, 255); // hvit tekst
inline const TDT4102::Color COLOR_PANEL   = TDT4102::Color(42, 42, 58);    // mørk panel-bakgrunn

// DEMO MAZES - orker ikke skrev ut hver gang
using Maze = std::vector<std::vector<char>>;

extern const std::vector<Maze> demo_mazes;

// Funksjoner jeg bruker i flere filer
void key_to_func(Grid& grid, Grid& backup, TDT4102::AnimationWindow& win, KeyboardKey s, PathfinderBase& p);
void draw_panel(TDT4102::AnimationWindow& win, const Grid& grid, const PathfinderBase& p);
void draw_line(TDT4102::AnimationWindow& win, int x, int& y, const std::string& text, TDT4102::Color c = COLOR_TXT);


// Alle tastene vi lytter etter i hovedløkken
inline const std::vector<KeyboardKey> keys =
{
    KeyboardKey::R, KeyboardKey::BACKSPACE,
    KeyboardKey::ESCAPE, KeyboardKey::ENTER,
    KeyboardKey::NUMPAD_PLUS, KeyboardKey::NUMPAD_MINUS
};

// Brukes til å vise tastenavnene i panelet for debugging
inline const std::map<KeyboardKey, std::string> key_names = {
    {KeyboardKey::R,           "R"},
    {KeyboardKey::BACKSPACE,   "BACKSPACE"},
    {KeyboardKey::ENTER,       "ENTER"},
    {KeyboardKey::NUMPAD_PLUS, "NUMPAD_PLUS"},
    {KeyboardKey::NUMPAD_MINUS,"NUMPAD_MINUS"},
    {KeyboardKey::ESCAPE,      "ESCAPE"}
};


// logikk for å ikke sende tastetrykk hver enste frame
class KeyHandler
{
private:
    std::map<KeyboardKey, bool> prev; // forrige frames tastestatus

public:
    // Sjekker om noen av de gitte tastene nettopp ble trykket ned
    std::optional<KeyboardKey> any_key_pressed(TDT4102::AnimationWindow& win, const std::vector<KeyboardKey>& keys)
    {
        for(KeyboardKey key : keys)
        {
            bool pressed = win.is_key_down(key);
            bool current = pressed && !prev[key]; // trykket nå men ikke forrige frame
            prev[key] = pressed;
            if(current)
                return key;
        }
        return std::nullopt; // ingen tast ble trykket
    }
};


// Håndterer museklikk når det gjelder å holde inne
struct MouseHandler
{
    TDT4102::Point pos;
    bool painting  = false; // holder på venstre museknapp og maler vegger
    bool erasing   = false; // holder på venstre museknapp og visker ut vegger
    bool prev_btn  = false; // forrige frames knappestatus
    std::optional<std::pair<int,int>> CELL; // cellen musa er over

    void determine_func(Grid& grid, TDT4102::AnimationWindow& win);
};

// Retninger for nabosøk: opp, ned, venstre, høyre
inline const std::vector<std::pair<int, int>> direc = {
    {-1, 0}, {1, 0},
    {0, -1}, {0, 1}
};

