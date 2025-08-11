#pragma once
#include <map>
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

#include "raylib.h"

#include "object3d.hpp"
#include "player.hpp"
#include "weather.hpp"

class World {
public:
    World();
    
    void load_world(std::string save_file, std::shared_ptr<Shader> shader);
    void save_world(std::string save_file) const;
    void reset_world();
    void set_alone(std::string current_user);

    std::string to_string() const;
    void from_string(std::string data, std::shared_ptr<Shader> shader);

    uint32_t load_object(std::shared_ptr<Object3d> object, std::shared_ptr<Shader> shader);
    void load_object(std::shared_ptr<Object3d> object, uint32_t id, std::shared_ptr<Shader> shader);
    void load_player(std::string username, std::shared_ptr<Shader> shader);
    void load_player(std::shared_ptr<Player> player, std::shared_ptr<Shader> shader);
    void update_object(uint32_t id, Vector3 position);
    void update_object(uint32_t id, Quaternion quaternion);
    uint32_t get_object_id(std::shared_ptr<Object3d> object);
    void remove_object(uint32_t id);

    const std::map<uint32_t, std::shared_ptr<Object3d>>& get_objects() const;
    const std::vector<std::shared_ptr<Player>>& get_players() const;
    const std::shared_ptr<Player> get_player(std::string username) const;
    std::shared_ptr<Weather> get_weather();
    std::shared_ptr<Object3d> get_sun();
    void update_sun();
private:
    uint32_t next_id_;
    std::shared_ptr<Weather> weather_;
    std::map<uint32_t, std::shared_ptr<Object3d>> objects_;
    std::vector<std::shared_ptr<Player>> players_;
    std::shared_ptr<Object3d> sun_;
    Vector3 spawn_point_;
};