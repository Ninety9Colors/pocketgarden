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

    json to_json() const;
    void from_json(const json& j);

    const uint32_t raycast_nearest(Ray ray) const;
    const uint32_t raycast_nearest(Ray ray, float maximum_distance, uint8_t type_flags) const;

    void clear_world();

    bool contains_object(uint32_t id) const;
    Vector3 get_object_position(uint32_t id) const;
    Quaternion get_object_quaternion(uint32_t id) const;
    BoundingBox get_object_bounding_box(uint32_t id) const;

    uint32_t load_object(std::unique_ptr<Object3d> object);
    void load_object(std::unique_ptr<Object3d> object, uint32_t object_id);
    void load_player(std::string username);
    void load_player(Player player);

    void move_object(uint32_t object_id, Vector3 new_position);
    void set_object_quaternion(uint32_t object_id, Quaternion quaternion);
    void rotate_object_axis(uint32_t object_id, Vector3 axis, float radians);
    void delete_object(uint32_t object_id);
    std::unique_ptr<Object3d> transfer_object(uint32_t object_id);

    void disconnect_players();
    const std::vector<Player>& get_players() const;
    const std::map<uint32_t,std::unique_ptr<Object3d>>& get_objects() const;
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