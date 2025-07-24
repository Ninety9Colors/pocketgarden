#pragma once
#include <memory>
#include <string>
#include <vector>

#include "network.hpp"
#include "object3d.hpp"
#include "player.hpp"
#include "world.hpp"

class Game {
public:
    Game();

    bool in_world() const;
    void poll_events();
    bool host(std::string current_user, std::string save_file, char* ip, char* port);
    bool join(std::string current_user, char* ip, char* port);

    void disconnect();

    const std::string& get_current_user();
    const std::shared_ptr<Player> get_current_player();

    std::shared_ptr<Network> get_network() const;
    std::shared_ptr<World> get_world() const;
private:
    bool in_world_;
    std::shared_ptr<World> world_;
    std::string current_user_;

    std::shared_ptr<Network> network_;
};