#include "Utils.h"
#include "AnimationWindow.h"
#include "Grid.h"
#include "Pathfinder.h"
#include <optional>

double animation_step = 0.01;
bool instant = false;
std::vector<std::string> g_algo_names;
std::vector<int>         g_path_lengths;


void execute_action(Action a, Grid& grid, Grid& backup, TDT4102::AnimationWindow& win)
{
    try {
        switch(a) {
            case Action::Reset:     backup = grid; grid.reset();                    break;
            case Action::Generate:  grid.generate_maze(); backup = grid;            break;
            case Action::Undo:      grid = backup;                                  break;
            case Action::Save:      grid.save_to_file(SAVE_FILE);                   break;
            case Action::Load:      backup = grid; grid.load_from_file(SAVE_FILE);  break;
            case Action::GridMinus: backup = grid; grid.resize(-GRID_STEP);         break;
            case Action::GridPlus:  backup = grid; grid.resize(GRID_STEP);          break;
        }
    }
    catch(const GridException& e)      { win.show_error_dialog(e.what()); }
    catch(const std::runtime_error& e) { win.show_error_dialog(e.what()); }
}


void key_to_func(Grid& grid, Grid& backup, TDT4102::AnimationWindow& win, KeyboardKey s, PathfinderBase& p)
{
    std::cout << "Tast ble trykket: " << key_names.at(s) << '\n';

    switch(s)
    {
        case KeyboardKey::R:        execute_action(Action::Reset,     grid, backup, win); grid.set_current_demo(-1); break;
        case KeyboardKey::BACKSPACE: execute_action(Action::Undo,      grid, backup, win); break;
        case KeyboardKey::S:        execute_action(Action::Save,       grid, backup, win); break;
        case KeyboardKey::NUMPAD_PLUS:  execute_action(Action::GridPlus,  grid, backup, win); break;
        case KeyboardKey::NUMPAD_MINUS: execute_action(Action::GridMinus, grid, backup, win); break;
        case KeyboardKey::L:        execute_action(Action::Load,       grid, backup, win); break;
        case KeyboardKey::G:        execute_action(Action::Generate,   grid, backup, win); break;

        case KeyboardKey::ENTER:
            backup = grid;
            p.find_path(grid, win);
            break;
        case KeyboardKey::ESCAPE:
            win.close();
            break;

        default:
            break;
    }
    
}


void MouseHandler::determine_func(Grid& grid, TDT4102::AnimationWindow& win)
{
    pos = win.get_mouse_coordinates();
    bool l_btn = win.is_left_mouse_button_down();
    bool r_btn = win.is_right_mouse_button_down();

    if(r_btn && !prev_btn)
    {
        CELL = grid.cell_at_pos(pos.x, pos.y);
        if(CELL)
            grid.right_click(CELL->first, CELL->second);
    }

    if(l_btn && !prev_btn)
    {
        CELL = grid.cell_at_pos(pos.x, pos.y);
        if(CELL)
        {
            CellState s = grid.get_cell(CELL->first, CELL->second);
            painting = (s == CellState::Empty);
            erasing  = (s == CellState::Wall);
        }
    }

    if(l_btn)
    {
        CELL = grid.cell_at_pos(pos.x, pos.y);
        if(CELL)
        {
            int r = CELL->first;
            int c = CELL->second;
            CellState s = grid.get_cell(r, c);
            if(painting && s != CellState::End && s != CellState::Start)
                grid.set_cell(r, c, CellState::Wall);
            if(erasing  && s != CellState::End && s != CellState::Start)
                grid.set_cell(r, c, CellState::Empty);
        }
    }
    else
    {
        painting = false;
        erasing  = false;
    }
    prev_btn = l_btn || r_btn;
}


void draw_line(TDT4102::AnimationWindow& win, int x, int& y, std::string text, TDT4102::Color c)
{
    win.draw_text({x, y}, text, c, FONT_SIZE);
    y += NEW_LINE;
}

void draw_panel(TDT4102::AnimationWindow& win, const Grid& grid, const PathfinderBase& p)
{
    win.draw_rectangle({PANEL_X_START, DEADSPACE}, PANEL_WIDTH, PANEL_INFO_HEIGHT, COLOR_PANEL);

    int x_1 = PANEL_X_START + PANEL_DEADSPACE_LARGE;
    int x_2 = PANEL_X_START + PANEL_WIDTH / 2;
    int y_1 = DEADSPACE + PANEL_DEADSPACE_LARGE;
    int y_2 = DEADSPACE + PANEL_DEADSPACE_LARGE;

    // VENSTRE: INFO
    draw_line(win, x_1, y_1, "=== INFO ===", TDT4102::Color::yellow);
    y_1 += PANEL_DEADPSACE_SMALL;

    std::string demo_str = (grid.get_current_demo() == -1) ? "ingen" : std::to_string(grid.get_current_demo());

    draw_line(win, x_1, y_1, "Algoritme:  " + p.name(), COLOR_TXT);
    draw_line(win, x_1, y_1, "Demo maze:  " + demo_str, COLOR_TXT);
    draw_line(win, x_1, y_1, "Grid:       " + std::to_string(grid.get_rows()) + " x " + std::to_string(grid.get_cols()), COLOR_TXT);

    std::string anim_t_str = std::to_string(static_cast<int>(animation_step * 1000)) + " ms";
    if(instant)
    {
        draw_line(win, x_1, y_1, "Anim. step: " + anim_t_str, TDT4102::Color(150, 150, 150));
        draw_line(win, x_1, y_1, "Instant:    JA", TDT4102::Color(100, 255, 100));
    }
    else
    {
        draw_line(win, x_1, y_1, "Anim. step: " + anim_t_str, COLOR_TXT);
        draw_line(win, x_1, y_1, "Instant:    NEI", TDT4102::Color(255, 100, 100));
    }

    // HØYRE: PATHLENGTH
    draw_line(win, x_2, y_2, "=== PATHLENGTH ===", TDT4102::Color(255, 220, 80));
    y_2 += PANEL_DEADPSACE_SMALL;

    for(int i = 0; i < static_cast<int>(g_algo_names.size()); i++)
    {
        std::string str_len = (g_path_lengths[i] < 0) ? "-" : std::to_string(g_path_lengths[i]);
        TDT4102::Color c = (g_algo_names[i] == p.name()) ? TDT4102::Color(255, 220, 80) : COLOR_TXT;
        draw_line(win, x_2, y_2, g_algo_names[i] + ": " + str_len, c);
    }

    // TEKST
    int y_maze = BTN_Y_START + 30;
    draw_line(win, x_1, y_maze, "Maze", COLOR_TXT);

    int y_file = BTN_Y_START + 165;
    draw_line(win, x_1, y_file, "File", COLOR_TXT);

    int y_grid = BTN_Y_START + 165;
    draw_line(win, PANEL_X_START + PANEL_WIDTH / 2 + 10, y_grid, "Grid size", COLOR_TXT);
}
