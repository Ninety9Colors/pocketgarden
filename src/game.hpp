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
    bool host(std::string current_user, std::string save_file, char* ip, char* port);
    bool join(std::string current_user, char* ip, char* port);
    void load_world(std::string save_file);
    void save_world(std::string save_file);
    void reset_world();

    void load_object(std::shared_ptr<Object3d> object);
    void load_player(std::string username);
    void load_player(std::string username, Vector3 position);
    const std::vector<std::shared_ptr<Object3d>>& get_objects();
    const std::vector<std::shared_ptr<Player>>& get_players();
    const std::shared_ptr<Player> get_player(std::string username);

    void sync_clients();
    bool is_host() const;
    bool is_online(std::string username) const;

    const std::string& get_current_user();

    void send_packet(std::string data, bool reliable) const;
private:
    bool in_world_;
    std::vector<std::shared_ptr<Object3d>> objects_;
    std::vector<std::shared_ptr<Player>> players_;
    Vector3 spawn_point_;
    std::string current_user_;

    std::unique_ptr<Network> network_;
};