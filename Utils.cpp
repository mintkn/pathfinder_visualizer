#include "Utils.h"
#include "AnimationWindow.h"
#include "Grid.h"
#include "Pathfinder.h"
#include <optional>

// Globale variabler - definert her, deklarert med extern i Utils.h
double animation_step = 0.01; // standard animasjonstid i sekunder
bool instant = false;          // om animasjonen skal hoppe over ventetiden
std::vector<std::string> g_algo_names;   // navnene på algoritmene (vises i panelet)
std::vector<int> g_path_lengths; // lengden på stien til hver algoritme (-1 = ikke kjørt)


// Utfører en handling basert på hvilken Action som ble sendt inn
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


// Kobler tastetrykk til riktig handling. håndterte tidliger all logikk selv, men nå sender den til execute_action()
void key_to_func(Grid& grid, Grid& backup, TDT4102::AnimationWindow& win, KeyboardKey s, PathfinderBase& p)
{
    switch(s)
    {
        case KeyboardKey::R:             execute_action(Action::Reset,     grid, backup, win); grid.set_current_demo(-1); break;
        case KeyboardKey::BACKSPACE:     execute_action(Action::Undo,      grid, backup, win); break;
        case KeyboardKey::NUMPAD_PLUS:   execute_action(Action::GridPlus,  grid, backup, win); break;
        case KeyboardKey::NUMPAD_MINUS:  execute_action(Action::GridMinus, grid, backup, win); break;

        case KeyboardKey::ENTER:
            // lagrer backup og starter søket
            backup = grid;
            p.find_path(grid, win);
            break;
        case KeyboardKey::ESCAPE:
            win.close(); // avslutter programmet
            break;

        default:
            break; // ignorer andre taster
    }

}


// Håndterer museinteraksjon med gridet
// Bestemmer om vi skal male vegger, viske ut, eller plassere start/slutt
void MouseHandler::determine_func(Grid& grid, TDT4102::AnimationWindow& win)
{
    pos = win.get_mouse_coordinates();
    bool l_btn = win.is_left_mouse_button_down();
    bool r_btn = win.is_right_mouse_button_down();

    // høyreklikk håndteres bare ved første klikk (ikke ved holdt inne)
    if(r_btn && !prev_btn)
    {
        CELL = grid.cell_at_pos(pos.x, pos.y);
        if(CELL)
            grid.right_click(CELL->first, CELL->second);
    }

    // ved første venstreklikk bestemmer vi om vi skal male eller viske
    if(l_btn && !prev_btn)
    {
        CELL = grid.cell_at_pos(pos.x, pos.y);
        if(CELL)
        {
            CellState s = grid.get_cell(CELL->first, CELL->second);
            painting = (s == CellState::Empty); // klikket på tomt - begynn å male vegger
            erasing  = (s == CellState::Wall);  // klikket på vegg - begynn å viske
        }
    }

    // mens venstre museknapp er holdt inne, fortsetter vi å male eller viske
    if(l_btn)
    {
        CELL = grid.cell_at_pos(pos.x, pos.y);
        if(CELL)
        {
            int r = CELL->first;
            int c = CELL->second;
            CellState s = grid.get_cell(r, c);
            // ikke overskriv start og slutt ved uhell
            if(painting && s != CellState::End && s != CellState::Start)
                grid.set_cell(r, c, CellState::Wall);
            if(erasing  && s != CellState::End && s != CellState::Start)
                grid.set_cell(r, c, CellState::Empty);
        }
    }
    else
    {
        // knappen er sluppet opp - avslutt maling/visking
        painting = false;
        erasing  = false;
    }
    prev_btn = l_btn || r_btn; // husk knappestatus til neste frame
}


// Hjelpefunksjon som tegner en linje med tekst og hopper til neste linje
// y er en referanse slik at den automatisk øker for neste kall
void draw_line(TDT4102::AnimationWindow& win, int x, int& y, const std::string& text, TDT4102::Color c)
{
    win.draw_text({x, y}, text, c, FONT_SIZE);
    y += NEW_LINE; // flytt y-koordinaten ned til neste linje
}

// Tegner hele informasjonspanelet på høyre side av vinduet
// Viser info om algoritme, gridstørrelse, animasjon og sti-lengder
void draw_panel(TDT4102::AnimationWindow& win, const Grid& grid, const PathfinderBase& p)
{
    // tegner bakgrunnsrektangelet for info-boksen
    win.draw_rectangle({PANEL_X_START, DEADSPACE}, PANEL_WIDTH, PANEL_INFO_HEIGHT, COLOR_PANEL);

    // x og y for venstre og høyre kolonne i panelet
    int x_1 = PANEL_X_START + PANEL_DEADSPACE_LARGE;
    int x_2 = PANEL_X_START + PANEL_WIDTH / 2;
    int y_1 = DEADSPACE + PANEL_DEADSPACE_LARGE;
    int y_2 = DEADSPACE + PANEL_DEADSPACE_LARGE;

    // VENSTRE: INFO - viser nåværende algoritme, demo og gridstørrelse
    draw_line(win, x_1, y_1, "=== INFO ===", TDT4102::Color::yellow);
    y_1 += PANEL_DEADSPACE_SMALL;

    // viser "ingen" hvis ingen demo er lastet
    std::string demo_str = (grid.get_current_demo() == -1) ? "ingen" : std::to_string(grid.get_current_demo());

    draw_line(win, x_1, y_1, "Algorithm:  " + p.name(), COLOR_TXT);
    draw_line(win, x_1, y_1, "Demo maze:  " + demo_str, COLOR_TXT);
    draw_line(win, x_1, y_1, "Grid:       " + std::to_string(grid.get_rows()) + " x " + std::to_string(grid.get_cols()), COLOR_TXT);

    // konverterer sekunder til millisekunder for visning
    std::string anim_t_str = std::to_string(static_cast<int>(animation_step * 1000)) + " ms";
    if(instant)
    {
        // animasjonstiden vises grå når instant er på (ikke relevant)
        draw_line(win, x_1, y_1, "Anim. step: " + anim_t_str, TDT4102::Color(150, 150, 150));
        draw_line(win, x_1, y_1, "Instant:    YES", TDT4102::Color(100, 255, 100));
    }
    else
    {
        draw_line(win, x_1, y_1, "Anim. step: " + anim_t_str, COLOR_TXT);
        draw_line(win, x_1, y_1, "Instant:    NO", TDT4102::Color(255, 100, 100));
    }

    // HØYRE: PATHLENGTH - viser sti-lengden for alle algoritmer
    draw_line(win, x_2, y_2, "=== PATHLENGTH ===", TDT4102::Color(255, 220, 80));
    y_2 += PANEL_DEADSPACE_SMALL;

    for(int i = 0; i < static_cast<int>(g_algo_names.size()); i++)
    {
        // "-" hvis algoritmen ikke har blitt kjørt ennå
        std::string str_len = (g_path_lengths[i] < 0) ? "-" : std::to_string(g_path_lengths[i]);
        // fremhever den valgte algoritmen med gul farge
        TDT4102::Color c = (g_algo_names[i] == p.name()) ? TDT4102::Color(255, 220, 80) : COLOR_TXT;
        draw_line(win, x_2, y_2, g_algo_names[i] + ": " + str_len, c);
    }

    // ETIKETT-TEKST - liten tekst over knappegruppene
    int y_maze = BTN_Y_START + 30;
    draw_line(win, x_1, y_maze, "Maze", COLOR_TXT);

    int y_file = BTN_Y_START + 165;
    draw_line(win, x_1, y_file, "File", COLOR_TXT);

    int y_anim = BTN_Y_START + 235;
    draw_line(win, x_1, y_anim, "Anim. step", COLOR_TXT);

    int y_grid = BTN_Y_START + 165;
    draw_line(win, PANEL_X_START + PANEL_WIDTH / 2 + 10, y_grid, "Grid size", COLOR_TXT);
}
