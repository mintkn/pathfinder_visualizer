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

enum class Action { Reset, Generate, Undo, Save, Load, GridMinus, GridPlus };

void execute_action(Action a, Grid& grid, Grid& backup, TDT4102::AnimationWindow& win);

// VINDU
inline constexpr int WIN_WIDTH = 1500;
inline constexpr int WIN_HEIGHT = 810;
inline constexpr int WIN_X = 50;
inline constexpr int WIN_Y = 50;
inline constexpr int DEADSPACE = 20;
inline constexpr int PANEL_X_START = 1000;
inline constexpr int PANEL_WIDTH = WIN_WIDTH - PANEL_X_START - DEADSPACE; // 480
inline constexpr int PANEL_INFO_HEIGHT = 195; // høyde på info-boksen
inline constexpr int WIDGET_Y_START = DEADSPACE + PANEL_INFO_HEIGHT + 20; // Dropdown starter her
inline constexpr int BTN_Y_START = WIDGET_Y_START + 250; // knapper starter her

// GRID
inline constexpr int GRID_MIN  = 5;
inline constexpr int GRID_MAX  = 100;
inline constexpr int GRID_STEP = 5;

extern double animation_step;
extern bool instant;
extern std::vector<std::string> g_algo_names;
extern std::vector<int> g_path_lengths;

// PANEL LAYOUT
inline constexpr int  FONT_SIZE = 16;
inline constexpr int  NEW_LINE = 22;
inline constexpr int  PANEL_DEADSPACE_SMALL = 4;
inline constexpr int  PANEL_DEADSPACE_LARGE = 10;
inline constexpr int  WIDGET_WIDTH = PANEL_WIDTH - 2 * PANEL_DEADSPACE_LARGE; // widget-bredde (460)


// LAGRING
constexpr std::string SAVE_FILE = "grid_save.txt";

// FARGER
inline const TDT4102::Color COLOR_EMPTY = TDT4102::Color(245, 245, 245);
inline const TDT4102::Color COLOR_WALL = TDT4102::Color(245, 230, 150);
inline const TDT4102::Color COLOR_START = TDT4102::Color(50, 255, 20);
inline const TDT4102::Color COLOR_END = TDT4102::Color(255, 50, 50);
inline const TDT4102::Color COLOR_VISITED = TDT4102::Color(75, 225, 255);
inline const TDT4102::Color COLOR_PATH = TDT4102::Color(200, 100, 255);
inline const TDT4102::Color COLOR_BG = TDT4102::Color(28, 28, 38);
inline const TDT4102::Color COLOR_GRID_LINE = TDT4102::Color(55, 55, 70);
inline const TDT4102::Color COLOR_TXT = TDT4102::Color(255, 255, 255);
inline const TDT4102::Color COLOR_PANEL = TDT4102::Color(42, 42, 58);

// DEMO MAZES
using Maze = std::vector<std::vector<char>>;

extern const std::vector<Maze> demo_mazes;

void key_to_func(Grid& grid, Grid& backup, TDT4102::AnimationWindow& win, KeyboardKey s, PathfinderBase& p);
void draw_panel(TDT4102::AnimationWindow& win, const Grid& grid, const PathfinderBase& p);
void draw_line(TDT4102::AnimationWindow& win, int x, int& y, const std::string& text, TDT4102::Color c = COLOR_TXT);


inline const std::vector<KeyboardKey> keys =
{
    KeyboardKey::R, KeyboardKey::BACKSPACE,
    KeyboardKey::ESCAPE, KeyboardKey::ENTER,
    KeyboardKey::NUMPAD_PLUS, KeyboardKey::NUMPAD_MINUS
};

inline const std::map<KeyboardKey, std::string> key_names = {
    {KeyboardKey::R,           "R"},
    {KeyboardKey::BACKSPACE,   "BACKSPACE"},
    {KeyboardKey::ENTER,       "ENTER"},
    {KeyboardKey::NUMPAD_PLUS, "NUMPAD_PLUS"},
    {KeyboardKey::NUMPAD_MINUS,"NUMPAD_MINUS"},
    {KeyboardKey::ESCAPE,      "ESCAPE"}
};


class KeyHandler
{
private:
    std::map<KeyboardKey, bool> prev;

public:
    std::optional<KeyboardKey> any_key_pressed(TDT4102::AnimationWindow& win, const std::vector<KeyboardKey>& keys)
    {
        for(KeyboardKey key : keys)
        {
            bool pressed = win.is_key_down(key);
            bool current = pressed && !prev[key];
            prev[key] = pressed;
            if(current)
                return key;
        }
        return std::nullopt;
    }
};


struct MouseHandler
{
    TDT4102::Point pos;
    bool painting  = false;
    bool erasing   = false;
    bool prev_btn  = false;
    std::optional<std::pair<int,int>> CELL;

    void determine_func(Grid& grid, TDT4102::AnimationWindow& win);
};


