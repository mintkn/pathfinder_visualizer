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
    srand(time(0));
    Grid grid(20, 20);
    Grid backup = grid;
    TDT4102::AnimationWindow win(WIN_X, WIN_Y, WIN_WIDTH, WIN_HEIGHT, "Pathfinding Visualizer");

    KeyHandler   kh;
    MouseHandler mouse;

    PathFinder   bfs;
    GreedyFinder greedy;
    ASTARFinder  afinder;
    RandomFinder randf;

    std::vector<PathfinderBase*> algos = { &bfs, &greedy, &afinder, &randf };
    int algo_idx = 0;
    g_algo_names   = {"BFS", "Greedy", "A*", "Random"};
    g_path_lengths = std::vector<int>(algos.size(), -1);
    std::vector<std::string> d_mazes = {"Demo Maze: 1", "Demo Maze: 2", "Demo Maze: 3", "Demo Maze: 4", "Demo Maze: 5"};
    constexpr int WX  = PANEL_X_START + PANEL_DEADSPACE_LARGE;  // 1010
    constexpr int WW  = WIDGET_WIDTH;                    // 460
    constexpr int BH  = 32;
    constexpr int B3W = (WW - 20) / 3;              // 146 — 1/3-bredde for 3-knapp-rad
    constexpr int B2W = 105;                         // knapp i halvrad

    // ALGORITME-VALG 
    TDT4102::DropdownList algo_list({WX, WIDGET_Y_START}, WW, 35, g_algo_names);
    algo_list.setCallback([&](){
        backup = grid;
        for(int i = 0; i < (int)g_algo_names.size(); i++)
            if(g_algo_names[i] == algo_list.getSelectedValue()) algo_idx = i;
    });

    TDT4102::DropdownList dem_mazes({WX, WIDGET_Y_START+55}, WW, 35, d_mazes);
    dem_mazes.setCallback([&](){
        try {
            for(int i = 0; i < (int)d_mazes.size(); i++) {
                if(d_mazes[i] == dem_mazes.getSelectedValue()) {
                    if(i >= (int)demo_mazes.size())
                        throw GridException("Demo-labyrint " + std::to_string(i+1) + " finnes ikke");
                    grid.load_demo_maze(demo_mazes[i]);
                    grid.set_current_demo(i);
                    backup = grid;
                }
            }
        }
        catch(const GridException& e)      { win.show_error_dialog(e.what()); }
        catch(const std::runtime_error& e) { win.show_error_dialog(e.what()); }
    });

    // MAZE: GENERATE | UNDO | RESET
    TDT4102::Button generate_btn({WX,              BTN_Y_START + 58}, B3W, BH, "GENERATE");
    TDT4102::Button undo_btn    ({WX + B3W + 10,   BTN_Y_START + 58}, B3W, BH, "UNDO");
    TDT4102::Button reset_btn   ({WX + 2*(B3W+10), BTN_Y_START + 58}, B3W, BH, "RESET");

    // RUN 
    TDT4102::Button run_btn({WX, BTN_Y_START + 95}, WW, 40, "RUN");

    // FILE: SAVE | LOAD   +   GRID SIZE: GRID- | GRID+ 
    TDT4102::Button save_btn ({WX,       BTN_Y_START + 193}, B2W, BH, "SAVE");
    TDT4102::Button load_btn ({WX + 115, BTN_Y_START + 193}, B2W, BH, "LOAD");
    TDT4102::Button minus_btn({WX + 240, BTN_Y_START + 193}, B2W, BH, "GRID -");
    TDT4102::Button plus_btn ({WX + 355, BTN_Y_START + 193}, B2W, BH, "GRID +");

    // ANIMASJON 
    TDT4102::Slider   anim_time   ({WX, BTN_Y_START + 232}, WW, 40, 0, 100, 10, 1);
    TDT4102::CheckBox instant_anim({WX, BTN_Y_START + 278}, WW, BH, "Instant");

    // FARGER
    bool run_requested = false;
    run_btn.setButtonColor(TDT4102::Color(50, 180, 80));
    reset_btn.setButtonColor(TDT4102::Color(210, 50, 50));
    generate_btn.setButtonColor(TDT4102::Color(120, 60, 200));
    undo_btn.setButtonColor(TDT4102::Color(210, 100, 20));
    save_btn.setButtonColor(TDT4102::Color(30, 120, 210));
    load_btn.setButtonColor(TDT4102::Color(30, 120, 210));
    minus_btn.setButtonColor(TDT4102::Color(65, 65, 80));
    plus_btn.setButtonColor(TDT4102::Color(65, 65, 80));


    // Widgets funksjoner
    run_btn.setCallback([&]() { run_requested = true; });
    reset_btn.setCallback([&](){ execute_action(Action::Reset,     grid, backup, win); });
    generate_btn.setCallback([&]() { execute_action(Action::Generate,  grid, backup, win); });
    undo_btn.setCallback([&]() { execute_action(Action::Undo,      grid, backup, win); });
    save_btn.setCallback([&]() { execute_action(Action::Save,      grid, backup, win); });
    load_btn.setCallback([&]() { execute_action(Action::Load,      grid, backup, win); });
    minus_btn.setCallback([&]() { execute_action(Action::GridMinus, grid, backup, win); });
    plus_btn.setCallback([&]() { execute_action(Action::GridPlus,  grid, backup, win); });
    anim_time.setCallback([&]() { animation_step = anim_time.getValue() / 1000.0; });
    instant_anim.setCallback([&]() { instant = !instant; });

    // Tegn widgets
    win.add(algo_list);
    win.add(generate_btn); win.add(undo_btn); win.add(reset_btn);
    win.add(run_btn);
    win.add(save_btn); win.add(load_btn);
    win.add(minus_btn); win.add(plus_btn);
    win.add(anim_time);
    win.add(instant_anim);
    win.add(dem_mazes);

    while(!win.should_close())
    {
        PathfinderBase& current = *algos[algo_idx];
 
        if(run_requested)
        {
            run_requested = false;
            backup = grid;
            try
            {
                current.find_path(grid, win);
                int len = static_cast<int>(current.get_final_path().size());
                g_path_lengths[algo_idx] = (len > 0) ? len : -1;
            }
            catch(const GridException& e)      { win.show_error_dialog(e.what()); }
            catch(const std::runtime_error& e) { win.show_error_dialog(e.what()); }
        }

        grid.draw(win);
        draw_panel(win, grid, current);
        win.next_frame();
        
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
        mouse.determine_func(grid, win);
    }
    return 0;
}
