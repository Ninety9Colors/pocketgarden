#include <assert.h>
#include <fstream>

#include "object/procedural/lily_flower.hpp"
#include "object/consistent/move_tool.hpp"
#include "object/consistent/sun_tool.hpp"
#include "object/consistent/rotate_tool.hpp"
#include "object/procedural/tapered_petal.hpp"
#include "world/world.hpp"
#include <cstdint>
#include "util.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

#include "logging.hpp"

constexpr float SUN_RADIUS = 100.0f;

World::World() : spawn_point_{0.0f,0.0f,0.0f},next_id_(1),sun_(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f,SUN_RADIUS,0.0f}, Vector3{1.0f,1.0f,1.0f}, 10.0f, WHITE),weather_(30.2672f, -97.7431f) {};

void World::load_world(std::string save_file) {
    clear_objects();
    std::ifstream file (save_file);
    if (file) {
        std::string data {};
        std::getline(file, data);
        from_string(data);
    } else {
        load_object(std::make_unique<Cube>(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,1.0f}, 1.0f, RED));
        auto flower = std::make_unique<LilyFlower>(Vector3{0.0f,0.0f,0.0f}, 1.0f);
        flower->generate_mesh();
        load_object(std::move(flower));
        load_object(std::make_unique<MoveTool>(Vector3{0.0f, 2.0f, 0.0f}, 1.0f));
        load_object(std::make_unique<SunTool>(Vector3{0.0f, 2.0f, 3.0f}, 1.0f));
        load_object(std::make_unique<RotateTool>(Vector3{0.0f, 2.0f, 4.0f}, 1.0f));
    }
}

void World::save_world(std::string save_file) const {
    std::ofstream file(save_file);
    file << to_string();
    file.close();
}

std::string World::to_string() const {
    std::string result = "World " + std::to_string(next_id_) + " (";
    for (const auto& p : objects_) {
        result += "(" + std::to_string(p.first) + " (" + p.second->to_string() + "))";
    }
    result += ")(";
    for (int i = 0; i < players_.size(); i++) {
        const auto& player = players_[i];
        result += "(" + player.to_string() + ")";
    }
    result += ") " + std::to_string(weather_.get_latitude()) + " " + std::to_string(weather_.get_longitude());
    return result;
}

void World::from_string(std::string data) {
    std::vector<std::string> split = split_string(data);
    next_id_ = std::stoi(split[1]);
    std::vector<std::string> object_data = split_string(split[2]);
    std::vector<std::string> player_data = split_string(split[3]);
    for (const std::string& data : object_data) {
        std::vector<std::string> object_split = split_string(data);
        std::string type = get_first_word(object_split[1]);
        std::unique_ptr<Object3d> object;
        if (type == "Cube") {
            object = std::make_unique<Cube>(object_split[1]);
        } else if (type == "MoveTool") {
            object = std::make_unique<MoveTool>(object_split[1]);
        } else if (type == "SunTool") {
            object = std::make_unique<SunTool>(object_split[1]);
        } else if (type == "RotateTool") {
            object = std::make_unique<RotateTool>(object_split[1]);
        } else if (type == "TaperedPetal") {
            object = std::make_unique<TaperedPetal>(object_split[1]);
            object->generate_mesh();
        } else if (type == "LilyFlower") {
            object = std::make_unique<LilyFlower>(object_split[1]);
            object->generate_mesh();
        }
        objects_[std::stoi(object_split[0])] = std::move(object);
    }
    for (const std::string& data : player_data) {
        load_player(Player(data));
    }
    weather_.set_location(std::stof(split[4]), std::stof(split[5]));
}

void World::clear_objects() {
    objects_.clear();
    players_.clear();
    next_id_ = 1;
}

uint32_t World::load_object(std::unique_ptr<Object3d> object) {
    objects_[next_id_++] = std::move(object);
    return next_id_-1;
}

void World::load_object(std::unique_ptr<Object3d> object, uint32_t object_id) {
    assert(objects_.find(object_id) == objects_.end());
    objects_[object_id] = std::move(object);
    next_id_ = std::max(object_id+1, next_id_); // maintain condition that no ids > next_id_ are used
}

void World::load_player(std::string username) {
    if (!get_player(username)) {
        players_.push_back(Player(username, spawn_point_));
    } else {
        WARN("Trying to load player that is already loaded: " + username);
    }
}

void World::load_player(Player player) {
    if (!get_player(player.get_username())) {
        players_.push_back(std::move(player));
    } else {
        WARN("Trying to load player that is already loaded: " + player.get_username());
    }
}

void World::move_object(uint32_t object_id, Vector3 new_position) {
    if(objects_.find(object_id) == objects_.end()) {
        WARN("Trying to move non existent object with id: " + std::to_string(object_id));
        return;
    }
    objects_[object_id]->set_position(new_position);
}

void World::rotate_object(uint32_t object_id, Quaternion quaternion) {
    if(objects_.find(object_id) == objects_.end()) {
        WARN("Trying to move non existent object with id: " + std::to_string(object_id));
        return;
    }
    objects_[object_id]->set_quaternion(quaternion);
}

void World::delete_object(uint32_t object_id) {
    if(objects_.find(object_id) == objects_.end()) {
        WARN("Trying to delete non existent object with id: " + std::to_string(object_id));
        return;
    }
    objects_.erase(object_id);
}

const std::vector<Player>& World::get_players() const {
    return players_;
}
std::optional<std::reference_wrapper<Player>> World::get_player(std::string username) {
    for (Player& player : players_) {
        if (player.get_username() == username)
            return std::ref<Player>(player);
    }
    WARN("Could not find player: " + username);
    return std::nullopt;
}

Weather World::get_weather() {
    return weather_;
}

const Cube& World::get_sun() const {
    return sun_;
}

void World::update_sun() {
    double azimuth = weather_.get_azimuth();
    double altitude = weather_.get_altitude();
    double x = std::sin(azimuth) * std::cos(altitude);
    double y = std::sin(altitude);
    double z = -std::cos(azimuth) * std::cos(altitude);
    double magnitude = std::sqrt(x*x + y*y + z*z);

    sun_.set_position(Vector3{(float)(x/magnitude)*SUN_RADIUS,(float)(y/magnitude)*SUN_RADIUS,(float)(z/magnitude)*SUN_RADIUS});
}