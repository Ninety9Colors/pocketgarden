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

class MoveTool : public Item {
public:
    MoveTool();
    MoveTool(std::string data);
    MoveTool(Vector3 position, float scale);

    void use(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) override;

    void prepare_drop(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<Player> user, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) override;

    bool in_use() const;

    std::string to_string() const override;
private:
    uint32_t held_id_;
    float holding_distance_;

    float speed_;
};