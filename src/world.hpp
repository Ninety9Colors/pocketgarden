#pragma once
#include <memory>
#include <string>
#include <vector>

#include "object3d.hpp"
#include "player.hpp"

class World {
public:
    World();
    
    void load_world(std::string save_file);
    void save_world(std::string save_file) const;
    void reset_world();
    void set_alone(std::string current_user);

    std::string to_string() const;
    void from_string(std::string data);

    void load_object(std::shared_ptr<Object3d> object);
    void load_player(std::string username);
    void load_player(std::shared_ptr<Player> player);
    const std::vector<std::shared_ptr<Object3d>>& get_objects() const;
    const std::vector<std::shared_ptr<Player>>& get_players() const;
    const std::shared_ptr<Player> get_player(std::string username) const;
private:
    std::vector<std::shared_ptr<Object3d>> objects_;
    std::vector<std::shared_ptr<Player>> players_;
    Vector3 spawn_point_;
};