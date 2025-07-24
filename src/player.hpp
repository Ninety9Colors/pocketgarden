#pragma once
#include <memory>
#include <string>
#include <vector>

#include "raylib.h"

#include "cube.hpp"
#include "object3d.hpp"

class Player {
public:
    Player(std::string username, Vector3 position);
    Player(std::string data);
    void draw(std::string current_user, int camera_mode) const;
    bool move(float dt, const Vector3& direction, int camera_mode, const std::vector<bool>& keybinds);
    void set_position(float x, float y, float z);
    void add_to_model(std::unique_ptr<Object3d>&& object);

    void on_join();
    void on_disconnect();
    bool is_online() const;

    std::string get_username();
    const Cube& get_hitbox();
    Vector3 get_position() const;

    std::string to_string() const;
    
private:
    std::string username_;
    float speed_;
    bool online_;

    Cube hitbox_;
    std::vector<std::unique_ptr<Object3d>> model_;
};