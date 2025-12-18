#pragma once
#include <memory>

#include "raylib.h"

class MainCamera {
public:
    MainCamera();
    void update(Vector3 position, Vector2 mouse_delta);
    void toggle_freecam();

    const Camera3D& get_camera() const;
    const Vector3& get_direction() const;
    const Vector3& get_position() const;
    int get_mode() const;

private:
    Camera3D camera_;
    int camera_mode_;
    Vector3 direction_;
};