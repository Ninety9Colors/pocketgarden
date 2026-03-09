#include "factory.hpp"

#include "event/event.hpp"
#include <object/consistent/cube.hpp>
#include <object/consistent/move_tool.hpp>
#include <object/consistent/rotate_tool.hpp>
#include <object/consistent/sun_tool.hpp>
#include <object/consistent/weather_tool.hpp>
#include <object/procedural/lily_flower.hpp>
#include <object/procedural/lily.hpp>
#include <object/procedural/tapered_petal.hpp>

#include "logging.hpp"

std::unique_ptr<Object3d> parse_object(const json& j) {
    INFO("Parsing object " + j.dump(4));
    std::string type = j.at("type");
    std::unique_ptr<Object3d> result = nullptr;
    if (type == "Cube") {
        result = std::make_unique<Cube>(j);
    } else if (type == "MoveTool") {
        result = std::make_unique<MoveTool>(j);
    } else if (type == "RotateTool") {
        result = std::make_unique<RotateTool>(j);
    } else if (type == "SunTool") {
        result = std::make_unique<SunTool>(j);
    } else if (type == "WeatherTool") {
        result = std::make_unique<WeatherTool>(j);
    } else if (type == "LilyFlower") {
        result = std::make_unique<LilyFlower>(j);
        result->generate_mesh();
    } else if (type == "Lily") {
        auto lily = std::make_unique<Lily>(j);
        lily->initialize();
        result = std::move(lily);
        result->generate_mesh();
    } else if (type == "TaperedPetal") {
        result = std::make_unique<TaperedPetal>(j);
        result->generate_mesh();
    } else if (type == "TaperedLeaf") {
        result = std::make_unique<TaperedLeaf>(j);
        result->generate_mesh();
    } else if (type == "null_item") {
        result = nullptr;
    } else {
        WARN("Could not parse object, type " + type + " was not found");
    }
    return std::move(result);
}

std::unique_ptr<Event> parse_event(const json& j) {
    std::string type = j.at("type");
    std::unique_ptr<Event> result = nullptr;
    if (type == "IAmHostEvent") {
        result = std::make_unique<IAmHostEvent>(j);
    } else if (type == "ConnectEvent") {
        result = std::make_unique<ConnectEvent>(j);
    } else if (type == "SyncEvent") {
        result = std::make_unique<SyncEvent>(j);
    } else if (type == "DisconnectEvent") {
        result = std::make_unique<DisconnectEvent>(j);
    } else if (type == "PlayerMoveEvent") {
        result = std::make_unique<PlayerMoveEvent>(j);
    } else if (type == "ObjectMoveEvent") {
        result = std::make_unique<ObjectMoveEvent>(j);
    } else if (type == "ObjectRotateEvent") {
        result = std::make_unique<ObjectRotateEvent>(j);
    } else if (type == "ObjectRemoveEvent") {
        result = std::make_unique<ObjectRemoveEvent>(j); 
    } else if (type == "ObjectLoadEvent") {
        result = std::make_unique<ObjectLoadEvent>(j); 
    } else if (type == "ItemPickupEvent") {
        result = std::make_unique<ItemPickupEvent>(j); 
    } else if (type == "ItemDropEvent") {
        result = std::make_unique<ItemDropEvent>(j);
    } else if (type == "WeatherUpdateEvent") {
        result = std::make_unique<WeatherUpdateEvent>(j);
    }
    if (result == nullptr)
        WARN("Could not parse event, type " + type + " was not found");
    return std::move(result);
}