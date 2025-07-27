#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "raylib.h"

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

    uint32_t load_object(std::shared_ptr<Object3d> object);
    void load_object(std::shared_ptr<Object3d> object, uint32_t id);
    void load_player(std::string username);
    void load_player(std::shared_ptr<Player> player);
    void update_object(uint32_t id, Vector3 position);
    uint32_t get_object_id(std::shared_ptr<Object3d> object);
    void remove_object(uint32_t id);
    const std::map<uint32_t, std::shared_ptr<Object3d>>& get_objects() const;
    const std::vector<std::shared_ptr<Player>>& get_players() const;
    const std::shared_ptr<Player> get_player(std::string username) const;
private:
    uint32_t next_id_;
    std::map<uint32_t, std::shared_ptr<Object3d>> objects_;
    std::vector<std::shared_ptr<Player>> players_;
    Vector3 spawn_point_;
};