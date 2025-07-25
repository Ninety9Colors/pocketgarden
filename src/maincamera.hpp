#pragma once
#include <memory>

#include "raylib.h"

#include "player.hpp"

class MainCamera {
public:
    MainCamera();
    void update(const std::shared_ptr<Player> player, Vector2 mouse_delta);
    void toggle_freecam();

    const Camera3D& get_camera() const;
    const Vector3& get_direction() const;
    const Vector3& get_position() const;
    int get_mode() const;

private:
    Camera3D camera_;
    int camera_mode_;
    float sensitivity_;
    Vector3 direction_;
};