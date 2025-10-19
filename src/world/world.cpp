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

constexpr float SUN_RADIUS = 100.0f;

World::World() {
    spawn_point_ = Vector3{0.0f,0.0f,0.0f};
    next_id_ = 1;
    weather_ = std::make_shared<Weather>(30.2672f, -97.7431f);
    sun_ = std::make_shared<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,1.0f}, 10.0f, WHITE);
};

void World::load_world(std::string save_file, std::shared_ptr<Shader> shader) {
    reset_world();
    std::ifstream file (save_file);
    if (file) {
        std::string data {};
        std::getline(file, data);
        from_string(data,shader);
    } else {
        load_object(std::make_shared<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,1.0f}, 1.0f, RED), shader);
        auto flower = std::make_shared<LilyFlower>(Vector3{0.0f,0.0f,0.0f}, 1.0f);
        flower->generate_mesh();
        load_object(flower,shader);
        load_object(std::make_shared<MoveTool>(Vector3{0.0f, 2.0f, 0.0f}, 1.0f), shader);
        load_object(std::make_shared<SunTool>(Vector3{0.0f, 2.0f, 3.0f}, 1.0f), shader);
        load_object(std::make_shared<RotateTool>(Vector3{0.0f, 2.0f, 4.0f}, 1.0f), shader);
    }
}

void World::save_world(std::string save_file) const {
    std::ofstream file(save_file);
    file << to_string();
    file.close();
}

void World::reset_world() {
    objects_.clear();
    players_.clear();
    next_id_ = 1;
}

void World::set_alone(std::string current_user) {
    for (const auto player : players_) {
        if (player->get_username() != current_user)
            player->on_disconnect();
    }
}

std::string World::to_string() const {
    std::string result = "World " + std::to_string(next_id_) + " (";
    for (const auto& p : objects_) {
        result += "(" + std::to_string(p.first) + " (" + p.second->to_string() + "))";
    }
    result += ")(";
    for (int i = 0; i < players_.size(); i++) {
        const auto& player = players_[i];
        result += "(" + player->to_string() + ")";
    }
    result += ") " + std::to_string(weather_->get_latitude()) + " " + std::to_string(weather_->get_longitude());
    return result;
}

void World::from_string(std::string data, std::shared_ptr<Shader> shader) {
    std::vector<std::string> split = split_string(data);
    next_id_ = std::stoi(split[1]);
    std::vector<std::string> object_data = split_string(split[2]);
    std::vector<std::string> player_data = split_string(split[3]);
    for (const std::string& data : object_data) {
        std::vector<std::string> object_split = split_string(data);
        std::string type = get_first_word(object_split[1]);
        std::shared_ptr<Object3d> object;
        if (type == "Cube") {
            object = std::make_shared<Cube>(object_split[1]);
        } else if (type == "MoveTool") {
            object = std::make_shared<MoveTool>(object_split[1]);
        } else if (type == "SunTool") {
            object = std::make_shared<SunTool>(object_split[1]);
        } else if (type == "RotateTool") {
            object = std::make_shared<RotateTool>(object_split[1]);
        } else if (type == "TaperedPetal") {
            object = std::make_shared<TaperedPetal>(object_split[1]);
            std::dynamic_pointer_cast<TaperedPetal>(object)->generate_mesh();
        } else if (type == "LilyFlower") {
            object = std::make_shared<LilyFlower>(object_split[1]);
            std::dynamic_pointer_cast<LilyFlower>(object)->generate_mesh();
        }
        object->set_shader(shader);
        objects_[std::stoi(object_split[0])] = object;
    }
    for (const std::string& data : player_data) {
        load_player(std::make_shared<Player>(data), shader);
    }
    weather_->set_location(std::stof(split[4]), std::stof(split[5]));
}

uint32_t World::load_object(std::shared_ptr<Object3d> object, std::shared_ptr<Shader> shader) {
    objects_[next_id_++] = std::move(object);
    objects_[next_id_-1]->set_shader(shader);
    return next_id_-1;
}

void World::load_object(std::shared_ptr<Object3d> object, uint32_t id, std::shared_ptr<Shader> shader) {
    assert(objects_.find(id) == objects_.end());
    objects_[id] = std::move(object);
    objects_[id]->set_shader(shader);
    next_id_ = std::max(id+1, next_id_);
}

void World::load_player(std::string username, std::shared_ptr<Shader> shader) {
    if (get_player(username) == nullptr) {
        players_.push_back(std::make_shared<Player>(username, spawn_point_));
        players_.back()->set_shader(shader);
    }
}

void World::load_player(std::shared_ptr<Player> player, std::shared_ptr<Shader> shader) {
    if (get_player(player->get_username()) == nullptr) {
        players_.push_back(std::move(player));
        players_.back()->set_shader(shader);
    }
}

void World::update_object(uint32_t id, Vector3 position) {
    if(objects_.find(id) == objects_.end()) return;
    objects_[id]->set_position(position);
}

void World::update_object(uint32_t id, Quaternion quaternion) {
    if(objects_.find(id) == objects_.end()) return;
    objects_[id]->set_quaternion(quaternion);
}

uint32_t World::get_object_id(std::shared_ptr<Object3d> object) {
    for (const auto& p : objects_) {
        if (object.get() == p.second.get()) {
            return p.first;
        }
    }
    return 0;
}

void World::remove_object(uint32_t id) {
    if (id != 0) {
        objects_.erase(id);
    }
}

const std::map<uint32_t, std::shared_ptr<Object3d>>& World::get_objects() const {
    return objects_;
}

const std::vector<std::shared_ptr<Player>>& World::get_players() const {
    return players_;
}
const std::shared_ptr<Player> World::get_player(std::string username) const {
    for (const auto& player : players_) {
        if (player->get_username() == username)
            return player;
    }
    return nullptr;
}

std::shared_ptr<Weather> World::get_weather() {
    return weather_;
}

std::shared_ptr<Object3d> World::get_sun() {
    return sun_;
}

void World::update_sun() {
    double azimuth = weather_->get_azimuth();
    double altitude = weather_->get_altitude();
    double x = std::sin(azimuth) * std::cos(altitude);
    double y = std::sin(altitude);
    double z = -std::cos(azimuth) * std::cos(altitude);
    double magnitude = std::sqrt(x*x + y*y + z*z);

    sun_->set_position(Vector3{(float)(x/magnitude)*SUN_RADIUS,(float)(y/magnitude)*SUN_RADIUS,(float)(z/magnitude)*SUN_RADIUS});
}