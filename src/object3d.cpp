#include "object3d.hpp"

#include "raylib.h"

std::shared_ptr<Shader> Object3d::get_shader() {
    return shader_;
}