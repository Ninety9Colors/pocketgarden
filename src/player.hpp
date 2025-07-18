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
    void draw(std::string current_user, int camera_mode) const;
    void on_join();
    void on_disconnect();

    std::string get_username();
    const Cube& get_hitbox();
    
private:
    std::string username_;
    bool online_;

    Cube hitbox_;
    std::vector<std::unique_ptr<Object3d>> model_;
};