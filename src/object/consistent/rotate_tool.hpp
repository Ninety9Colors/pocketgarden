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
    RotateTool(std::string data);
    RotateTool(Vector3 position, float scale);

    void use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) override;
    void draw() const override;
    void draw_offset(float x, float y, float z) const override;

    void prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) override;

    bool in_use() const;

    std::string to_string() const override;
private:
    uint32_t held_id_;
    std::weak_ptr<Object3d> held_item_;

    Vector3 axis_;

    float rotate_speed_; // radians per second
};