#pragma once
#include <memory>
#include <string>
#include <vector>

#include "raylib.h"

#include "object/object3d.hpp"
#include "player/maincamera.hpp"
#include "player/player.hpp"
#include "world/world.hpp"
#include "event/event.hpp"
#include "game.hpp"

class SunTool : public Item {
public:
    SunTool();
    SunTool(const json& j);
    SunTool(Quaternion quaternion, Vector3 position, float scale);

    void generate_mesh() override;

    void use(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) override;
    void on_drop(Game& game, std::string username, const std::vector<bool>& keybinds, float dt) override;

    json to_json() const override;
    void from_json(const json& j) override;
private:
    int time_offset_;
    float speed_;
    Color color_;
};