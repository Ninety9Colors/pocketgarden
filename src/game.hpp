#pragma once
#include <memory>
#include <string>
#include <vector>

#include "object3d.hpp"
#include "player.hpp"

class Game {
public:
    Game(std::string current_user);
    void load_world(std::string save_file);
    void save_world(std::string save_file);

    void add_object(std::unique_ptr<Object3d> object);
    void add_player(std::shared_ptr<Player> player);
    const std::vector<std::unique_ptr<Object3d>>& get_objects();
    const std::vector<std::shared_ptr<Player>>& get_players();
    const std::shared_ptr<Player> get_player(std::string username);

    const std::string& get_current_user();
private:
    std::vector<std::unique_ptr<Object3d>> objects_;
    std::vector<std::shared_ptr<Player>> players_;
    std::string current_user_;
};