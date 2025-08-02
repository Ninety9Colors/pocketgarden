#include <assert.h>
#include <iostream>
#include <limits>
#include <cmath>
#include "sun_tool.hpp"

#include "raylib.h"

#include "cube.hpp"
#include "maincamera.hpp"
#include "util.hpp"
#include <ctime>

SunTool::SunTool() : position_{0.0f, 0.0f, 0.0f}, scale_(1.0f) {
    model_ = LoadModelFromMesh(GenMeshSphere(0.25f,8,8));
    speed_ = 1.0f;
    color_ = Color{200,200,0,255};
    time_offset_ = 0;
}

SunTool::SunTool(std::string data) {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "SunTool" && split.size() == 6);
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    time_offset_ = std::stof(split[4]);
    scale_ = std::stof(split[5]);
    model_ = LoadModelFromMesh(GenMeshSphere(0.25f,8,8));
    speed_ = 1.0f;
    color_ = Color{200,200,0,255};
    time_offset_ = 0;
}

SunTool::SunTool(Vector3 position, float scale) : position_(position), scale_(scale) {
    model_ = LoadModelFromMesh(GenMeshSphere(0.25f,8,8));
    speed_ = 1.0f;
    color_ = Color{200,200,0,255};
    time_offset_ = 0;
}

SunTool::~SunTool() {
    UnloadModel(model_);
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

void SunTool::draw() const {
    DrawModel(model_,position_,scale_,color_);
}

void SunTool::draw_offset(float x, float y, float z) const {
    DrawModel(model_,Vector3{position_.x+x,position_.y+y,position_.z+z},scale_,color_);
}

void SunTool::set_shader(std::shared_ptr<Shader> shader) {
    shader_ = shader;
    model_.materials[MATERIAL_MAP_DIFFUSE].shader = *shader_;
}

void SunTool::prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    time_offset_ = 0;
    uint64_t timestamp = std::time(nullptr);
    world->get_weather()->update_sun(timestamp);
    world->update_sun();
    event_buffer["WeatherUpdateEvent"] = std::make_shared<WeatherUpdateEvent>(world->get_weather()->get_weather_id());
}

void SunTool::set_x(float new_x) {
    position_.x = new_x;
}
void SunTool::set_y(float new_y) {
    position_.y = new_y;
}
void SunTool::set_z(float new_z) {
    position_.z = new_z;
}
float SunTool::get_x() const {
    return position_.x;
}
float SunTool::get_y() const {
    return position_.y;
}
float SunTool::get_z() const {
    return position_.z;
}

BoundingBox SunTool::get_bounding_box() const {
    BoundingBox box = GetModelBoundingBox(model_);
    box.max = Vector3{box.max.x*scale_ + position_.x, box.max.y*scale_ + position_.y, box.max.z*scale_ + position_.z};
    box.min = Vector3{box.min.x*scale_ + position_.x, box.min.y*scale_ + position_.y, box.min.z*scale_ + position_.z};
    return box;
}

std::string SunTool::to_string() const {
    std::string result = "SunTool " + 
        std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
        std::to_string(time_offset_) + " " +
        std::to_string(scale_);
    return result;
}