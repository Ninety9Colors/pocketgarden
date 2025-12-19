#pragma once
#include <cstdint>
#include <vector>
#include <queue>

#include "game.hpp"
#include "object/object3d.hpp"
#include "world/world.hpp"

class Application {
public:
    Application();

    void run(Game& game);
    void display_menu(Game& game);
    void display_scoreboard(const std::vector<Player>& players);
    void draw_objects(const std::map<uint32_t, std::unique_ptr<Object3d>>& objects) const;
    void draw_players(std::string current_user, const std::vector<std::shared_ptr<Player>>& players) const;
    void exit();
private:
    std::shared_ptr<Shader> shader_default_;
    std::shared_ptr<Shader> shader_light_source_;

    char ip_[16];
    char port_[6];
    char username_[16];
    bool ip_focus_;
    bool port_focus_;
    bool username_focus_;
};