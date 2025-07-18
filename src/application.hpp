#pragma once
#include <vector>

#include "game.hpp"
#include "maincamera.hpp"
#include "object3d.hpp"

class Application {
public:
    Application();

    void run(Game& game);
    void draw_objects(const std::vector<std::unique_ptr<Object3d>>& objects);
    void draw_players(std::string current_user, const std::vector<std::shared_ptr<Player>>& players, const MainCamera& main_camera);
    void exit();
};