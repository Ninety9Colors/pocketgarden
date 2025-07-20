#pragma once
#include <vector>

#include "game.hpp"
#include "maincamera.hpp"
#include "object3d.hpp"

class Application {
public:
    Application();

    void run(Game& game);
    void display_menu(Game& game, char* ip, char* port, bool& ip_focus, bool& port_focus);
    void display_scoreboard(const std::vector<std::shared_ptr<Player>>& players);
    void draw_objects(const std::vector<std::shared_ptr<Object3d>>& objects);
    void draw_players(std::string current_user, const std::vector<std::shared_ptr<Player>>& players, const MainCamera& main_camera);
    void exit();
};