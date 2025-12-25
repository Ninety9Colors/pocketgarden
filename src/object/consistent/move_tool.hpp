#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

#include "raylib.h"

#include "object/object3d.hpp"
#include "player/player.hpp"
#include "game.hpp"
#include "event/event.hpp"

class MoveTool : public Item {
public:
    MoveTool();
    MoveTool(const json& j);
    MoveTool(Quaternion quaternion, Vector3 position, float scale);

    void use(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) override;
    void on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) override;

    void generate_mesh() override;

    json to_json() const override;
    void from_json(const json& j) override;
private:
    uint32_t held_id_;
    float holding_distance_;

    float speed_;
};