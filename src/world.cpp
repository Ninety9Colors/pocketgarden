#include <assert.h>
#include <fstream>

#include "move_tool.hpp"
#include "world.hpp"
#include "util.hpp"

World::World() {
    spawn_point_ = Vector3{0.0f,0.0f,0.0f};
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
        load_object(std::make_shared<MoveTool>(Vector3{0.0f, 3.0f, 0.0f}, 1.0f));
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
    std::string result = "World (";
    for (int i = 0; i < objects_.size(); i++) {
        const auto& object = objects_[i];
        result += "(" + object->to_string() + ")";
    }
    result += ")(";
    for (int i = 0; i < players_.size(); i++) {
        const auto& player = players_[i];
        result += "(" + player->to_string() + ")";
    }
    result += ")";
    return result;
}

void World::from_string(std::string data) {
    std::vector<std::string> split = split_string(data);
    std::vector<std::string> object_data = split_string(split[1]);
    std::vector<std::string> player_data = split_string(split[2]);
    for (const std::string& data : object_data) {
        std::string first = get_first_word(data);
        if (first == "Cube") {
            load_object(std::make_shared<Cube>(data));
        } else if (first == "MoveTool") {
            load_object(std::make_shared<MoveTool>(data));
        }
    }
    for (const std::string& data : player_data) {
        load_player(std::make_shared<Player>(data));
    }
}

void World::load_object(std::shared_ptr<Object3d> object) {
    objects_.push_back(std::move(object));
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

const std::vector<std::shared_ptr<Object3d>>& World::get_objects() const {
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