#include <assert.h>
#include <iostream>
#include <limits>
#include <cmath>
#include <map>
#include "object/consistent/weather_tool.hpp"

#include "raylib.h"

#include "object/consistent/cube.hpp"
#include "player/maincamera.hpp"
#include "logging.hpp"

#include <ctime>

WeatherTool::WeatherTool() : Item(), current_id_(800), color_(255,165,0,255) {
    generate_mesh();
    update_matrix();
}

WeatherTool::WeatherTool(const json& j) {
    from_json(j);
    generate_mesh();
    update_matrix();
};

WeatherTool::WeatherTool(Quaternion quaternion, Vector3 position, float scale) : Item(quaternion, position,scale), current_id_(800), color_(255,165,0,255) {
    generate_mesh();
    update_matrix();
}

void WeatherTool::generate_mesh() {
    mesh_ = GenMeshTorus(0.5f,0.5f,16,16);
    material_.maps[MATERIAL_MAP_DIFFUSE].color = color_;
}

json WeatherTool::to_json() const {
    json j = {
        {"type","WeatherTool"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"current_id",current_id_},
        {"scale",scale_},
        {"color",{{"r",color_.r},{"g",color_.g},{"b",color_.b},{"a",color_.a}}},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}}
    };
    return j;
}

void WeatherTool::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    current_id_ = j.at("current_id");
    scale_ = j.at("scale");
    color_ = {j.at("color")["r"],j.at("color")["g"],j.at("color")["b"],j.at("color")["a"]};
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
}

void WeatherTool::use(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    std::map<int,int> next_id {};
    next_id[800] = 500;
    next_id[500] = 600;
    next_id[600] = 700;
    next_id[700] = 800;

    if (keybinds[6]) {
        current_id_ = next_id[current_id_];
        game.get_world().get_weather().set_weather_id(current_id_);
        game.get_world().get_weather().update_weather_transform(game);
        game.queue_event_send(std::make_unique<WeatherUpdateEvent>(current_id_,0));
        INFO("Player " + username + " clicked with WeatherTool, new weather id: " + std::to_string(current_id_));
    }
}

void WeatherTool::on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) {
    
}