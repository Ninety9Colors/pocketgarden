#include <assert.h>
#include <cstdint>
#include <memory>

#include "event/event.hpp"
#include "game.hpp"
#include "player/maincamera.hpp"
#include "object/consistent/move_tool.hpp"
#include "object/consistent/sun_tool.hpp"
#include "object/consistent/rotate_tool.hpp"
#include "object/procedural/tapered_petal.hpp"
#include "player/player.hpp"
#include "util/factory.hpp"
#include "object/object3d.hpp"
#include "object/procedural/lily_flower.hpp"

#include "raylib.h"
#include "raymath.h"

#include "logging.hpp"

Event::Event() {}

IAmHostEvent::IAmHostEvent(std::string host_username) : host_username_(host_username) {}
IAmHostEvent::IAmHostEvent(const json& j) {from_json(j);}
IAmHostEvent::~IAmHostEvent() {}

json IAmHostEvent::to_json() const {
    json j = {
        {"type","IAmHostEvent"},
        {"host_username",host_username_}
    };
    return j;
}

void IAmHostEvent::from_json(const json& j) {
    host_username_ = j.at("host_username");
}

bool IAmHostEvent::reliable() const {
    return true;
};

void IAmHostEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    if (is_host) {
    } else {
        assert(game.get_world().get_player(host_username_) != std::nullopt);
        game.get_world().get_player(host_username_)->get().set_online(true);
    }
}

ConnectEvent::ConnectEvent(std::string username) : username_(username) {}
ConnectEvent::ConnectEvent(const json& j) {from_json(j);}
ConnectEvent::~ConnectEvent() {}

json ConnectEvent::to_json() const {
    json j = {
        {"type","ConnectEvent"},
        {"username",username_}
    };
    return j;
}

void ConnectEvent::from_json(const json& j) {
    username_ = j.at("username");
}

bool ConnectEvent::reliable() const {
    return true;
};
void ConnectEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    if (is_host) {
        game.get_world().load_player(username_);
        game.get_world().get_player(username_)->get().set_online(true);
        SyncEvent sync {game.get_world()};
        IAmHostEvent server_connect {game.get_current_username()};
        WeatherUpdateEvent weather_update {game.get_world().get_weather().get_weather_id()};
        game.get_network().send_packet(sync.to_json(), sync.reliable(), username_);
        game.get_network().send_packet(weather_update.to_json(), weather_update.reliable(), username_);
        game.get_network().send_packet(server_connect.to_json(),server_connect.reliable(),username_);

        game.get_network().send_packet_excluding(to_json(),reliable(),username_);
    } else {
        game.get_world().load_player(username_);
        game.get_world().get_player(username_)->get().set_online(true);            
    }
}

DisconnectEvent::DisconnectEvent(std::string username) : username_(username) {}
DisconnectEvent::DisconnectEvent(const json& j) {from_json(j);}
DisconnectEvent::~DisconnectEvent() {}

json DisconnectEvent::to_json() const {
    json j = {
        {"type","DisconnectEvent"},
        {"username",username_}
    };
    return j;
}

void DisconnectEvent::from_json(const json& j) {
    username_ = j.at("username");
}

bool DisconnectEvent::reliable() const {
    return true;
};
void DisconnectEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    assert(game.get_world().get_player(username_) != std::nullopt);
    if (is_host) {
        game.get_world().get_player(username_)->get().set_online(false);
        game.get_network().send_packet(to_json(), reliable());
    } else {
        game.get_world().get_player(username_)->get().set_online(false);
    }
}

SyncEvent::SyncEvent(const World& world) : world_json_(world.to_json()) {}
SyncEvent::SyncEvent(const json& j) {from_json(j);}
SyncEvent::~SyncEvent() {};

json SyncEvent::to_json() const {
    json j = {
        {"type","SyncEvent"},
        {"data",world_json_}
    };
    return j;
};

void SyncEvent::from_json(const json& j) {
    world_json_ = j.at("data");
}

bool SyncEvent::reliable() const {
    return true;
};

void SyncEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    game.get_world().clear_world();
    game.get_world().from_json(world_json_);
};

PlayerMoveEvent::PlayerMoveEvent(std::string username, Vector3 position) : username_(username), position_(std::make_unique<Vector3>(position)) {}
PlayerMoveEvent::PlayerMoveEvent(const json& j) {from_json(j);}
PlayerMoveEvent::~PlayerMoveEvent(){};

json PlayerMoveEvent::to_json() const {
    json j = {
        {"type","PlayerMoveEvent"},
        {"username",username_},
        {"position",{{"x",position_->x},{"y",position_->y},{"z",position_->z}}}
    };
    return j;
}

void PlayerMoveEvent::from_json(const json& j) {
    username_ = j.at("username");
    position_ = std::make_unique<Vector3>(j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]);
}

bool PlayerMoveEvent::reliable() const {
    return false;
};
void PlayerMoveEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    game.get_world().get_player(username_)->get().set_position(*position_);
    if (is_host) {
        game.get_network().send_packet_excluding(to_json(),reliable(),username_);
    }
}

ObjectMoveEvent::ObjectMoveEvent(uint32_t id, Vector3 position, std::string sender) : id_(id), position_(std::make_unique<Vector3>(position)), sender_(sender) {};
ObjectMoveEvent::ObjectMoveEvent(const json& j) {from_json(j);}
ObjectMoveEvent::~ObjectMoveEvent() {};

json ObjectMoveEvent::to_json() const {
    json j = {
        {"type","ObjectMoveEvent"},
        {"id",id_},
        {"position",{{"x",position_->x},{"y",position_->y},{"z",position_->z}}},
        {"sender",sender_}
    };
    return j;
}

void ObjectMoveEvent::from_json(const json& j) {
    id_ = j.at("id");
    position_ = std::make_unique<Vector3>(j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]);
    sender_ = j.at("sender");
}

bool ObjectMoveEvent::reliable() const {return false;}

void ObjectMoveEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    game.get_world().move_object(id_, *position_);
    if (is_host) {
        game.get_network().send_packet_excluding(to_json(), reliable(), sender_);
    }
}

ObjectRotateEvent::ObjectRotateEvent(uint32_t id, Quaternion quaternion, std::string sender) : id_(id), quaternion_(std::make_unique<Quaternion>(quaternion)), sender_(sender) {};
ObjectRotateEvent::ObjectRotateEvent(const json& j) {from_json(j);}
ObjectRotateEvent::~ObjectRotateEvent() {};
json ObjectRotateEvent::to_json() const {
    json j = {
        {"type","ObjectRotateEvent"},
        {"id",id_},
        {"quaternion",{{"x",quaternion_->x},{"y",quaternion_->y},{"z",quaternion_->z},{"w",quaternion_->w}}},
        {"sender",sender_}
    };
    return j;
}

void ObjectRotateEvent::from_json(const json& j) {
    id_ = j.at("id");
    quaternion_ = std::make_unique<Quaternion>(j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]);
    sender_ = j.at("sender");
}

bool ObjectRotateEvent::reliable() const {return false;}

void ObjectRotateEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    game.get_world().set_object_quaternion(id_, *quaternion_);
    if (is_host) {
        game.get_network().send_packet_excluding(to_json(), reliable(), sender_);
    }
}

ObjectRemoveEvent::ObjectRemoveEvent(uint32_t id, std::string sender) : id_(id), sender_(sender) {}
ObjectRemoveEvent::ObjectRemoveEvent(const json& j) {from_json(j);}
ObjectRemoveEvent::~ObjectRemoveEvent() {}
json ObjectRemoveEvent::to_json() const {
    json j = {
        {"type","ObjectRemoveEvent"},
        {"id",id_},
        {"sender",sender_}
    };
    return j;
}

void ObjectRemoveEvent::from_json(const json& j) {
    id_ = j.at("id");
    sender_ = j.at("sender");
}

bool ObjectRemoveEvent::reliable() const {return true;}

void ObjectRemoveEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    game.get_world().delete_object(id_);
    if (is_host)
        game.get_network().send_packet_excluding(to_json(), reliable(), sender_);
}

ObjectLoadEvent::ObjectLoadEvent(uint32_t id, json object_json, std::string sender) : id_(id), object_json_(object_json), sender_(sender) {}
ObjectLoadEvent::ObjectLoadEvent(const json& j) {from_json(j);}
ObjectLoadEvent::~ObjectLoadEvent() {}
json ObjectLoadEvent::to_json() const {
    assert(object_json_.is_object());
    json j = {
        {"type","ObjectLoadEvent"},
        {"id",id_},
        {"data",object_json_},
        {"sender",sender_}
    };
    return j;
}

void ObjectLoadEvent::from_json(const json& j) {
    id_ = j.at("id");
    object_json_ = j.at("data");
    sender_ = j.at("sender");
}

bool ObjectLoadEvent::reliable() const {
    return true;
}
void ObjectLoadEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    game.get_world().load_object(std::move(parse_object(object_json_)),id_);
    if (is_host)
        game.get_network().send_packet_excluding(to_json(), reliable(), sender_);
}

ItemPickupEvent::ItemPickupEvent(json item_json, std::string username) : item_json_(item_json), username_(username) {}
ItemPickupEvent::ItemPickupEvent(const json& j) {from_json(j);}
ItemPickupEvent::~ItemPickupEvent() {}
json ItemPickupEvent::to_json() const {
    assert(item_json_.is_object());
    json j = {
        {"type","ItemPickupEvent"},
        {"data",item_json_},
        {"player",username_}
    };
    return j;
}
void ItemPickupEvent::from_json(const json& j) {
    item_json_ = j.at("data");
    username_ = j.at("player");
}
bool ItemPickupEvent::reliable() const {
    return true;
}
void ItemPickupEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    auto player = game.get_world().get_player(username_);
    assert(player != std::nullopt);
    player->get().set_item(std::move(parse_object(item_json_)));
    if (is_host)
        game.get_network().send_packet_excluding(to_json(), reliable(), username_);
}

ItemDropEvent::ItemDropEvent(std::string username) : username_(username) {}
ItemDropEvent::ItemDropEvent(const json& j) {from_json(j);}
ItemDropEvent::~ItemDropEvent() {}

json ItemDropEvent::to_json() const {
    json j = {
        {"type","ItemDropEvent"},
        {"username",username_}
    };
    return j;
}
void ItemDropEvent::from_json(const json& j) {
    username_ = j.at("username");
}
bool ItemDropEvent::reliable() const {
    return true;
}
void ItemDropEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    auto player = game.get_world().get_player(username_);
    assert(player != std::nullopt);
    player->get().drop_item(game,username_,keybinds,dt);
    if (is_host)
        game.get_network().send_packet_excluding(to_json(), reliable(), username_);
}

WeatherUpdateEvent::WeatherUpdateEvent(int id, int timestamp_offset) : weather_id_(id), timestamp_offset_(timestamp_offset) {}
WeatherUpdateEvent::WeatherUpdateEvent(const json& j) {from_json(j);}
WeatherUpdateEvent::~WeatherUpdateEvent() {}
json WeatherUpdateEvent::to_json() const {
    json j = {
        {"type","WeatherUpdateEvent"},
        {"weather_id",weather_id_},
        {"timestamp_offset",timestamp_offset_}
    };
    return j;
}
void WeatherUpdateEvent::from_json(const json& j) {
    weather_id_ = j.at("weather_id");
    timestamp_offset_ = j.at("timestamp_offset");
}
bool WeatherUpdateEvent::reliable() const {
    return true;
}
void WeatherUpdateEvent::receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) {
    if (!is_host)
        game.get_world().get_weather().set_weather_id(weather_id_);
    game.get_world().get_weather().update_sun(current_timestamp+timestamp_offset_);
    game.get_world().update_sun();
    game.get_world().get_weather().update_weather_transform(game);
}