#pragma once
#include <memory>
#include <string>
#include <vector>

#include "network.hpp"
#include "object3d.hpp"
#include "player.hpp"

class Game {
public:
    Game();

    bool in_world() const;
    void poll_events();
    void host(std::string current_user, std::string save_file, char* ip, char* port);
    void join(std::string current_user, char* ip, char* port);
    void load_world(std::string save_file);
    void save_world(std::string save_file);

    void add_object(std::unique_ptr<Object3d> object);
    void add_player(std::shared_ptr<Player> player);
    const std::vector<std::unique_ptr<Object3d>>& get_objects();
    const std::vector<std::shared_ptr<Player>>& get_players();
    const std::shared_ptr<Player> get_player(std::string username);

    const std::string& get_current_user();
private:
    bool in_world_;
    std::vector<std::unique_ptr<Object3d>> objects_;
    std::vector<std::shared_ptr<Player>> players_;
    std::string current_user_;

    Network network_;
};