#include <assert.h>
#include <fstream>

#include "object/procedural/lily_flower.hpp"
#include "object/consistent/move_tool.hpp"
#include "object/consistent/sun_tool.hpp"
#include "object/consistent/weather_tool.hpp"
#include "object/consistent/rotate_tool.hpp"
#include "object/procedural/tapered_petal.hpp"
#include "world/world.hpp"
#include "util/factory.hpp"
#include "application.hpp"
#include <cstdint>

#include <algorithm>
#include <cmath>
#include <iostream>

#include "logging.hpp"

constexpr float SUN_RADIUS = 100.0f;

World::World() : spawn_point_{0.0f,0.0f,0.0f},next_id_(1),sun_(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f,SUN_RADIUS,0.0f}, Vector3{1.0f,1.0f,1.0f}, 10.0f, WHITE),weather_(30.2672f, -97.7431f) {
    sun_.set_material(LoadMaterialDefault());
    INFO("Default initialized world");
};

void World::load_world(std::string save_file) {
    clear_world();
    std::ifstream file (save_file);
    if (file) {
        INFO("Parsing json file...");
        json j = json::parse(file);
        INFO("Parsed!");
        from_json(j);
    } else {
        INFO("Could not find save file, loading default world...");
        load_object(std::make_unique<Cube>(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f,0.0f,0.0f},Vector3{1.0f,1.0f,1.0f}, 1.0f, RED));
        auto flower = std::make_unique<LilyFlower>(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f,0.0f,0.0f}, 1.0f);
        flower->generate_mesh();
        load_object(std::move(flower));
        load_object(std::make_unique<MoveTool>(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f, 2.0f, 0.0f}, 1.0f));
        load_object(std::make_unique<SunTool>(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f, 2.0f, 3.0f}, 1.0f));
        load_object(std::make_unique<RotateTool>(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f, 2.0f, 4.0f}, 1.0f));
        load_object(std::make_unique<WeatherTool>(Quaternion(0.0f,0.0f,0.0f,1.0f),Vector3{0.0f, 2.0f, 5.0f}, 1.0f));
        INFO("Default world loaded!");
    }
    file.close();
}

void World::save_world(std::string save_file) const {
    std::ofstream file(save_file);
    file << to_json().dump(4);
    file.close();
}

json World::to_json() const {
    json j = {
        {"type","World"},
        {"next_id",next_id_},
        {"spawn_point",{{"x",spawn_point_.x},{"y",spawn_point_.y},{"z",spawn_point_.z}}},
        {"objects",{}},
        {"players",{}},
        {"weather",{{"latitude",weather_.get_latitude()},{"longitude",weather_.get_longitude()}}}
    };
    for (const auto& p : objects_)
        j.at("objects")[std::to_string(p.first)] = p.second->to_json();
    for (const auto& player : players_)
        j.at("players").push_back(player.to_json());
    return j;
}

void World::from_json(const json& j) {
    DEBUG("Trying to create world from json: " + j.dump(4));
    clear_world();
    next_id_ = j.at("next_id");
    spawn_point_ = Vector3{j.at("spawn_point")["x"],j.at("spawn_point")["y"],j.at("spawn_point")["z"]};
    for (const auto& item : j.at("objects").items())
        load_object(parse_object(item.value()),std::stoi(item.key()));
    for (const auto& player : j.at("players")) {
        Player p = Player(player);
        load_player(p);
    }
    weather_.set_location(j.at("weather")["latitude"], j.at("weather")["longitude"]);
}

const uint32_t World::raycast_nearest(Ray ray) const {
    uint8_t type_flags =  static_cast<uint8_t>(ObjectType::NONE);
    float maximum_distance = std::numeric_limits<float>::infinity();
    uint32_t id = 0;
    float min_distance = std::numeric_limits<float>::infinity();
    for (const auto& p : objects_) {
        if ((p.second->get_type() & type_flags) != type_flags)
            continue;
        RayCollision c = GetRayCollisionBox(ray, p.second->get_bounding_box());
        if (c.hit) {
            float d = c.distance;
            if (d < min_distance && d <= maximum_distance) {
                id = p.first;
                min_distance = d;
            }
        }
    }
    return id;
}

const uint32_t World::raycast_nearest(Ray ray, float maximum_distance, uint8_t type_flags) const {
    uint32_t id = 0;
    float min_distance = std::numeric_limits<float>::infinity();
    for (const auto& p : objects_) {
        if ((p.second->get_type() & type_flags) != type_flags)
            continue;
        RayCollision c = GetRayCollisionBox(ray, p.second->get_bounding_box());
        if (c.hit) {
            float d = c.distance;
            if (d < min_distance && d <= maximum_distance) {
                id = p.first;
                min_distance = d;
            }
        }
    }
    return id;
}

void World::clear_world() {
    objects_.clear();
    players_.clear();
    next_id_ = 1;
}

bool World::contains_object(uint32_t id) const {
    return objects_.find(id) != objects_.end();
}

Vector3 World::get_object_position(uint32_t id) const {
    if (contains_object(id)) {
        return objects_.at(id)->get_position();
    } else {
        WARN("Tried to get position of nonexistent object " + std::to_string(id));
        return Vector3{};
    }
}

Quaternion World::get_object_quaternion(uint32_t id) const {
    if (contains_object(id)) {
        return objects_.at(id)->get_quaternion();
    } else {
        WARN("Tried to get position of nonexistent object " + std::to_string(id));
        return Quaternion{};
    }
}

BoundingBox World::get_object_bounding_box(uint32_t id) const {
    if (contains_object(id)) {
        return objects_.at(id)->get_bounding_box();
    } else {
        WARN("Tried to get bounding box of nonexistent object " + std::to_string(id));
        return BoundingBox{};
    }
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
    if (get_player(username) == std::nullopt) {
        INFO("Loading player with username: " + username);
        players_.push_back(Player(username, spawn_point_));
    } else {
        WARN("Trying to load player that is already loaded: " + username);
    }
}

void World::load_player(Player player) {
    if (get_player(player.get_username()) == std::nullopt) {
        INFO("Loaded player with username: " + player.get_username());
        players_.push_back(player);
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

void World::set_object_quaternion(uint32_t object_id, Quaternion quaternion) {
    if(objects_.find(object_id) == objects_.end()) {
        WARN("Trying to move non existent object with id: " + std::to_string(object_id));
        return;
    }
    objects_[object_id]->set_quaternion(quaternion);
}

void World::rotate_object_axis(uint32_t object_id, Vector3 axis, float radians) {
    if(objects_.find(object_id) == objects_.end()) {
        WARN("Trying to move non existent object with id: " + std::to_string(object_id));
        return;
    }
    objects_[object_id]->rotate_axis(axis,radians);
}

void World::delete_object(uint32_t object_id) {
    if(objects_.find(object_id) == objects_.end()) {
        WARN("Trying to delete non existent object with id: " + std::to_string(object_id));
        return;
    }
    objects_.erase(object_id);
}

std::unique_ptr<Object3d> World::transfer_object(uint32_t object_id) {
    if (objects_.find(object_id) == objects_.end())
        return nullptr;
    std::unique_ptr<Object3d> object = std::move(objects_[object_id]);
    objects_.erase(object_id);
    return std::move(object);
}

void World::disconnect_players() {
    for (Player& p : players_)
        p.set_online(false);
}

const std::vector<Player>& World::get_players() const {
    return players_;
}
const std::map<uint32_t,std::unique_ptr<Object3d>>& World::get_objects() const {
    return objects_;
}
std::optional<std::reference_wrapper<Player>> World::get_player(std::string username) {
    for (Player& player : players_) {
        if (player.get_username() == username)
            return std::ref<Player>(player);
    }
    return std::nullopt;
}

Weather& World::get_weather() {
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

    // Pass new lighting information to shader
    Shader shader_default = Application::get_shader_default();
    int sun_position_loc = GetShaderLocation(shader_default,"sunPos");
    int sun_color_loc = GetShaderLocation(shader_default,"sunColor");
    int ambient_loc = GetShaderLocation(shader_default,"ambient");

    const Vector3 sun_position = sun_.get_position();
    float sun_pos[3] = {sun_position.x, sun_position.y, sun_position.z};
    SetShaderValue(shader_default, sun_position_loc, sun_pos, SHADER_UNIFORM_VEC3);
    float sun_color[4] = {1.0f,1.0f,(std::pow(std::max(sun_position.y,0.0f),2)/10000.0f),1.0f};
    SetShaderValue(shader_default, sun_color_loc, sun_color, SHADER_UNIFORM_VEC4);

    float ambient_level = (std::pow(std::max(sun_position.y,0.0f),2)/10000.0f)*0.5f + 0.25f;
    float ambient[4] = {ambient_level,ambient_level,ambient_level,1.0f};
    SetShaderValue(shader_default, ambient_loc, ambient, SHADER_UNIFORM_VEC4);
}