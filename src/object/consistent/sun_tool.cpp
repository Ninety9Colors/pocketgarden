#include <assert.h>
#include <iostream>
#include <limits>
#include <cmath>
#include "object/consistent/sun_tool.hpp"

#include "raylib.h"

#include "object/consistent/cube.hpp"
#include "player/maincamera.hpp"

#include <ctime>

SunTool::SunTool() : Item(), speed_(1.0f), color_(200,200,0,255), time_offset_(0) {
    generate_mesh();
    update_matrix();
}

SunTool::SunTool(const json& j) {
    from_json(j);
    generate_mesh();
    update_matrix();
};

SunTool::SunTool(Quaternion quaternion, Vector3 position, float scale) : Item(quaternion, position,scale), speed_(1.0f), color_(200,200,0,255), time_offset_(0) {
    generate_mesh();
    update_matrix();
}

void SunTool::generate_mesh() {
    mesh_ = GenMeshSphere(0.25f,8,8);
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
}

json SunTool::to_json() const {
    json j = {
        {"type","SunTool"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"speed",speed_},
        {"time_offset",time_offset_},
        {"scale",scale_},
        {"color",{{"r",color_.r},{"g",color_.g},{"b",color_.b},{"a",color_.a}}},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}}
    };
    return j;
}

void SunTool::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    speed_ = j.at("speed");
    time_offset_ = j.at("time_offset");
    scale_ = j.at("scale");
    color_ = {j.at("color")["r"],j.at("color")["g"],j.at("color")["b"],j.at("color")["a"]};
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
}

void SunTool::use(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    if (!keybinds[7] && !keybinds[8])
        return;
    int64_t current_timestamp = std::time(nullptr);
    if (keybinds[7])
        time_offset_ += (int)(600*speed_);
    else if (keybinds[8])
        time_offset_ -= (int)(600*speed_);
    game.get_world().get_weather().update_sun(current_timestamp+time_offset_);
    game.get_world().update_sun();
    game.queue_event_send(std::make_unique<WeatherUpdateEvent>(game.get_world().get_weather().get_weather_id(),time_offset_));
}

void SunTool::on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    time_offset_ = 0;
    uint64_t timestamp = std::time(nullptr);
    game.get_world().get_weather().update_sun(timestamp);
    game.get_world().update_sun();
    game.queue_event_send(std::make_unique<WeatherUpdateEvent>(game.get_world().get_weather().get_weather_id()));
}