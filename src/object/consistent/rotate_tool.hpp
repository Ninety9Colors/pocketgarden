#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

#include "raylib.h"

#include "object/object3d.hpp"
#include "player/maincamera.hpp"
#include "player/player.hpp"
#include "world/world.hpp"
#include "event/event.hpp"

class RotateTool : public Item {
public:
    RotateTool();
    RotateTool(const json& j);
    RotateTool(Quaternion quaternion, Vector3 position, float scale);

    void draw(Game& game, Material material_) const override;
    void draw(Game& game, Matrix transform, Material material_) const override;

    void use(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) override;
    void on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) override;

    void generate_mesh() override;

    json to_json() const override;
    void from_json(const json& j);
private:
    uint32_t held_id_;
    Vector3 axis_;

    float rotate_speed_; // radians per second
};