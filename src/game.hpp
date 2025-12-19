#pragma once
#include <memory>
#include <string>
#include <functional>
#include <vector>

#include "network/network.hpp"
#include "object/object3d.hpp"
#include "player/player.hpp"
#include "world/world.hpp"
#include "player/maincamera.hpp"

class Game {
public:
    Game();

    bool in_world() const;
    bool host(std::string current_user, std::string save_file, char* ip, char* port);
    bool join(std::string current_user, char* ip, char* port);

    void close_game();

    const std::string& get_current_username() const;
    std::optional<std::reference_wrapper<Player>> get_current_player();

    void poll_events();
    void tick(uint64_t current_timestamp);
    void update_current_player(std::deque<std::unique_ptr<Event>>& event_buffer, const std::vector<bool>& keybinds, float dt);

    const Network& get_network() const;
    const World& get_world() const;
private:
    bool in_world_;
    World world_;
    std::string current_username_;

    std::deque<std::unique_ptr<Event>> event_buffer_;

    MainCamera main_camera_;
    Network network_;
};