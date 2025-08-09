#pragma once
#include <cstdint>
#include <vector>

#include "game.hpp"
#include "maincamera.hpp"
#include "object3d.hpp"
#include "world.hpp"

class Application {
public:
    Application();

    void tick(std::map<std::string, std::shared_ptr<Event>>& event_buffer, Game& game);

    void run(Game& game);
    void display_menu(Game& game);
    void display_scoreboard(const std::vector<std::shared_ptr<Player>>& players);
    void draw_objects(const std::map<uint32_t, std::shared_ptr<Object3d>>& objects);
    void draw_players(std::string current_user, const std::vector<std::shared_ptr<Player>>& players, const MainCamera& main_camera);
    void exit();

    std::map<std::string, std::shared_ptr<Event>>& get_event_buffer();
private:
    std::map<std::string, std::shared_ptr<Event>> event_buffer_;
    std::shared_ptr<Shader> shader_default_;
    std::shared_ptr<Shader> shader_light_source_;

    char ip_[16];
    char port_[6];
    char username_[16];
    bool ip_focus_;
    bool port_focus_;
    bool username_focus_;
};