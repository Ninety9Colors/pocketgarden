#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <cstdint>
#include <functional>
#include <vector>

#include "raylib.h"

#include "object/object3d.hpp"
#include "player/player.hpp"
#include "world/weather.hpp"

class World {
public:
    World();
    
    void load_world(std::string save_file);
    void save_world(std::string save_file) const;

    std::string to_string() const;
    void from_string(std::string data);

    void clear_objects();
    uint32_t load_object(std::unique_ptr<Object3d> object);
    void load_object(std::unique_ptr<Object3d> object, uint32_t object_id);
    void load_player(std::string username);
    void load_player(Player player);
    void move_object(uint32_t object_id, Vector3 new_position);
    void rotate_object(uint32_t object_id, Quaternion quaternion);
    void delete_object(uint32_t object_id);

    const std::vector<Player>& get_players() const;
    std::optional<std::reference_wrapper<Player>> get_player(std::string username);
    Weather get_weather();
    const Cube& get_sun() const;
    void update_sun();
private:
    uint32_t next_id_;
    Weather weather_;
    std::map<uint32_t, std::unique_ptr<Object3d>> objects_;
    std::vector<Player> players_;
    Cube sun_;
    Vector3 spawn_point_;
};