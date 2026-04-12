#include "AnimationWindow.h"
#include "Grid.h"
#include "GridException.h"
#include "Utils.h"
#include "Pathfinder.h"
#include "widgets/DropdownList.h"
#include "widgets/Button.h"
#include "widgets/Slider.h"
#include "widgets/CheckBox.h"
#include <string>
#include <iostream>
#include <vector>




int main()
{
    srand(time(0)); // initialiserer tilfeldig tallgenerator

    // lager et 20x20 grid og en kopi som backup (brukes til Undo)
    Grid grid(20, 20);
    Grid backup = grid;
    TDT4102::AnimationWindow win(WIN_X, WIN_Y, WIN_WIDTH, WIN_HEIGHT, "Pathfinding Visualizer");

    KeyHandler   kh;
    MouseHandler mouse;

    // lager ett objekt av hver algoritme
    PathFinder   bfs;
    GreedyFinder greedy;
    ASTARFinder  afinder;
    RandomFinder randf;

    // samler alle algoritmer i en vektor med pekere til basisklassen
    std::vector<PathfinderBase*> algos = { &bfs, &greedy, &afinder, &randf };
    int algo_idx = 0; // hvilken algoritme som er valgt nå
    g_algo_names   = {"BFS", "Greedy", "A*", "Random"};
    g_path_lengths = std::vector<int>(algos.size(), -1); // -1 betyr "ikke kjørt ennå"

    // navnene på demo-labyrintene som vises i nedtrekksmenyen
    std::vector<std::string> d_mazes = {"Demo Maze: 1", "Demo Maze: 2", "Demo Maze: 3", "Demo Maze: 4", "Demo Maze: 5"};

    // layout-konstanter for widget-plasseringen
    constexpr int WX  = PANEL_X_START + PANEL_DEADSPACE_LARGE;  // 1010
    constexpr int WW  = WIDGET_WIDTH;                    // 460
    constexpr int BH  = 32;
    constexpr int B3W = (WW - 20) / 3;              // 146 — 1/3-bredde for 3-knapp-rad
    constexpr int B2W = 105;                         // knapp i halvrad

    // ALGORITME-VALG - nedtrekksmeny for å velge søkealgoritme
    TDT4102::DropdownList algo_list({WX, WIDGET_Y_START}, WW, 35, g_algo_names);
    algo_list.setCallback([&](){
        // finner indeksen til den valgte algorytmen
        for(int i = 0; i < (int)g_algo_names.size(); i++)
            if(g_algo_names[i] == algo_list.getSelectedValue()) algo_idx = i;
    });

    // nedtrekksmeny for å velge en ferdiglaget demo-labyrint
    TDT4102::DropdownList dem_mazes({WX, WIDGET_Y_START+55}, WW, 35, d_mazes);
    dem_mazes.setCallback([&](){
        try {
            for(int i = 0; i < (int)d_mazes.size(); i++) {
                if(d_mazes[i] == dem_mazes.getSelectedValue()) {
                    if(i >= (int)demo_mazes.size())
                        throw GridException("Demo-labyrint " + std::to_string(i+1) + " finnes ikke");
                    grid.load_demo_maze(demo_mazes[i]);
                    grid.set_current_demo(i);
                    backup = grid; // oppdaterer backup etter lasting
                }
            }
        }
        catch(const GridException& e)      { win.show_error_dialog(e.what()); }
        catch(const std::runtime_error& e) { win.show_error_dialog(e.what()); }
    });

    // MAZE: tre knapper for å generere, angre og nullstille labyrinten
    TDT4102::Button generate_btn({WX,              BTN_Y_START + 58}, B3W, BH, "GENERATE");
    TDT4102::Button undo_btn    ({WX + B3W + 10,   BTN_Y_START + 58}, B3W, BH, "UNDO");
    TDT4102::Button reset_btn   ({WX + 2*(B3W+10), BTN_Y_START + 58}, B3W, BH, "RESET");

    // RUN - stor knapp for å starte søket
    TDT4102::Button run_btn({WX, BTN_Y_START + 95}, WW, 40, "RUN");

    // FILE og GRID SIZE - knapper for fil og størrelsesendring
    TDT4102::Button save_btn ({WX,       BTN_Y_START + 193}, B2W, BH, "SAVE");
    TDT4102::Button load_btn ({WX + 115, BTN_Y_START + 193}, B2W, BH, "LOAD");
    TDT4102::Button minus_btn({WX + 240, BTN_Y_START + 193}, B2W, BH, "GRID -");
    TDT4102::Button plus_btn ({WX + 355, BTN_Y_START + 193}, B2W, BH, "GRID +");

    // ANIMASJON - checkbox for instant og slider for animasjonshastighet
    TDT4102::CheckBox instant_anim({WX, BTN_Y_START + 250}, WW, BH, "Instant");
    TDT4102::Slider   anim_time   ({WX+77, BTN_Y_START + 252}, WW-68, 40, 0, 100, 10, 1);


    // FARGER - setter farger på knappene
    bool run_requested = false;
    run_btn.setButtonColor(TDT4102::Color(50, 180, 80));     // grønn
    reset_btn.setButtonColor(TDT4102::Color(210, 50, 50));   // rød
    generate_btn.setButtonColor(TDT4102::Color(120, 60, 200)); // lilla
    undo_btn.setButtonColor(TDT4102::Color(210, 100, 20));   // oransje
    save_btn.setButtonColor(TDT4102::Color(30, 120, 210));   // blå
    load_btn.setButtonColor(TDT4102::Color(30, 120, 210));   // blå
    minus_btn.setButtonColor(TDT4102::Color(65, 65, 80));    // mørk grå
    plus_btn.setButtonColor(TDT4102::Color(65, 65, 80));     // mørk grå


    // Kobler callbacks til knappene - lambda-funksjoner som kalles ved klikk
    run_btn.setCallback([&]() { run_requested = true; }); // setter bare flagget, selve kjøringen skjer i løkken
    reset_btn.setCallback([&](){ execute_action(Action::Reset,     grid, backup, win); });
    generate_btn.setCallback([&]() { execute_action(Action::Generate,  grid, backup, win); });
    undo_btn.setCallback([&]() { execute_action(Action::Undo,      grid, backup, win); });
    save_btn.setCallback([&]() { execute_action(Action::Save,      grid, backup, win); });
    load_btn.setCallback([&]() { execute_action(Action::Load,      grid, backup, win); });
    minus_btn.setCallback([&]() { execute_action(Action::GridMinus, grid, backup, win); });
    plus_btn.setCallback([&]() { execute_action(Action::GridPlus,  grid, backup, win); });
    anim_time.setCallback([&]() { animation_step = anim_time.getValue() / 1000.0; }); // konverterer ms til sekunder
    instant_anim.setCallback([&]() { instant = !instant; }); // toggler instant-modus

    // Registrerer alle widgets i vinduet
    win.add(algo_list);
    win.add(generate_btn); win.add(undo_btn); win.add(reset_btn);
    win.add(run_btn);
    win.add(save_btn); win.add(load_btn);
    win.add(minus_btn); win.add(plus_btn);
    win.add(anim_time);
    win.add(instant_anim);
    win.add(dem_mazes);

    // Hovedløkken - kjører til vinduet lukkes
    while(!win.should_close())
    {
        // henter referanse til den valgte algoritmen
        PathfinderBase& current = *algos[algo_idx];

        // kjør søket hvis RUN-knappen ble trykket
        if(run_requested)
        {
            run_requested = false;
            backup = grid; // ta backup før søket slik at man kan Undo
            try
            {
                current.find_path(grid, win);
                // lagrer sti-lengden slik at den vises i panelet
                int len = static_cast<int>(current.get_final_path().size());
                g_path_lengths[algo_idx] = (len > 0) ? len : -1;
            }
            catch(const GridException& e)      { win.show_error_dialog(e.what()); }
            catch(const std::runtime_error& e) { win.show_error_dialog(e.what()); }
        }

        // tegner gridet og sidepanelet for hver frame
        grid.draw(win);
        draw_panel(win, grid, current);
        win.next_frame(); // viser den ferdig tegnede framen

        // sjekker om noen taster ble trykket
        if(auto key = kh.any_key_pressed(win, keys))
        {
            try
            {
                key_to_func(grid, backup, win, key.value(), current);
            }
            catch(const GridException& e)
            {
                win.show_error_dialog(e.what());
            }
            catch(const std::runtime_error& e)
            {
                win.show_error_dialog(e.what());
            }
        }
        // håndterer museklikk og maling
        mouse.determine_func(grid, win);
    }
    return 0;
}
