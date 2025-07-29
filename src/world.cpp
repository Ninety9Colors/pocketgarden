#include <assert.h>
#include <fstream>

#include "move_tool.hpp"
#include "world.hpp"
#include "util.hpp"
#include <algorithm>

World::World() {
    spawn_point_ = Vector3{0.0f,0.0f,0.0f};
    next_id_ = 1;
    weather_ = std::make_shared<Weather>(30.2672f, -97.7431f);
    sun_ = std::make_shared<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,1.0f}, 100.0f, YELLOW);
};

void World::load_world(std::string save_file) {
    reset_world();
    std::ifstream file (save_file);
    if (file) {
        std::string data {};
        std::getline(file, data);
        from_string(data);
    } else {
        load_object(std::make_shared<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,1.0f}, 1.0f, RED));
        load_object(std::make_shared<MoveTool>(Vector3{0.0f, 2.0f, 0.0f}, 1.0f));
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
}

void World::set_alone(std::string current_user) {
    for (const auto player : players_) {
        if (player->get_username() != current_user)
            player->on_disconnect();
    }
}

std::string World::to_string() const{
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

void World::from_string(std::string data) {
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
        }
        objects_[std::stoi(object_split[0])] = object;
    }
    for (const std::string& data : player_data) {
        load_player(std::make_shared<Player>(data));
    }
    weather_->set_location(std::stof(split[4]), std::stof(split[5]));
}

uint32_t World::load_object(std::shared_ptr<Object3d> object) {
    objects_[next_id_++] = std::move(object);
    return next_id_-1;
}

void World::load_object(std::shared_ptr<Object3d> object, uint32_t id) {
    assert(objects_.find(id) == objects_.end());
    objects_[id] = std::move(object);
    next_id_ = std::max(id+1, next_id_);
}

void World::load_player(std::string username) {
    if (get_player(username) == nullptr) {
        players_.push_back(std::make_shared<Player>(username, spawn_point_));
    }
}

void World::load_player(std::shared_ptr<Player> player) {
    if (get_player(player->get_username()) == nullptr) {
        players_.push_back(std::move(player));
    }
}

void World::update_object(uint32_t id, Vector3 position) {
    assert(objects_.find(id) != objects_.end());
    objects_[id]->set_x(position.x);
    objects_[id]->set_y(position.y);
    objects_[id]->set_z(position.z);
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