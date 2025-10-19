#include <assert.h>
#include <iostream>
#include <limits>
#include <cmath>
#include "object/consistent/sun_tool.hpp"

#include "raylib.h"

#include "object/consistent/cube.hpp"
#include "player/maincamera.hpp"
#include "util.hpp"
#include <ctime>

SunTool::SunTool() : Item() {
    UnloadMesh(mesh_);
    mesh_ = GenMeshSphere(0.25f,8,8);
    speed_ = 1.0f;
    color_ = Color{200,200,0,255};
    time_offset_ = 0;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
    update_matrix();
}

SunTool::SunTool(std::string data) : Item() {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "SunTool" && split.size() == 6);
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    time_offset_ = std::stof(split[4]);
    scale_ = std::stof(split[5]);
    UnloadMesh(mesh_);
    mesh_ = GenMeshSphere(0.25f,8,8);
    speed_ = 1.0f;
    color_ = Color{200,200,0,255};
    time_offset_ = 0;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
    update_matrix();
}

SunTool::SunTool(Vector3 position, float scale) : Item(position,scale) {
    UnloadMesh(mesh_);
    mesh_ = GenMeshSphere(0.25f,8,8);
    speed_ = 1.0f;
    color_ = Color{200,200,0,255};
    time_offset_ = 0;
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
    update_matrix();
}

void SunTool::use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    if (keybinds[7]) {
        int64_t current_timestamp = std::time(nullptr);
        time_offset_ += (int)(600*speed_);
        world->get_weather()->update_sun(current_timestamp+time_offset_);
        world->update_sun();
        event_buffer["WeatherUpdateEvent"] = std::make_shared<WeatherUpdateEvent>(world->get_weather()->get_weather_id(), time_offset_);
    } else if (keybinds[8]) {
        int64_t current_timestamp = std::time(nullptr);
        time_offset_ -= (int)(600*speed_);
        world->get_weather()->update_sun(current_timestamp+time_offset_);
        world->update_sun();
        event_buffer["WeatherUpdateEvent"] = std::make_shared<WeatherUpdateEvent>(world->get_weather()->get_weather_id(), time_offset_);
    }
}

void SunTool::prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    time_offset_ = 0;
    uint64_t timestamp = std::time(nullptr);
    world->get_weather()->update_sun(timestamp);
    world->update_sun();
    event_buffer["WeatherUpdateEvent"] = std::make_shared<WeatherUpdateEvent>(world->get_weather()->get_weather_id());
}

std::string SunTool::to_string() const {
    std::string result = "SunTool " + 
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(time_offset_) + " " +
        std::to_string(scale_);
    return result;
}