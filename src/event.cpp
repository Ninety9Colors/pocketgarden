#include <assert.h>
#include <cstdint>
#include <memory>

#include "raylib.h"

#include "event.hpp"
#include "game.hpp"
#include "maincamera.hpp"
#include "move_tool.hpp"
#include "sun_tool.hpp"
#include "rotate_tool.hpp"
#include "player.hpp"
#include "util.hpp"
#include "object3d.hpp"

IAmHostEvent::IAmHostEvent(std::string username) : username_(username) {}
IAmHostEvent::~IAmHostEvent() {}

std::string IAmHostEvent::make_packet() const {
    std::string packet = "IAmHostEvent " + username_;
    return packet;
}

bool IAmHostEvent::reliable() const {
    return true;
};

void IAmHostEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    if (network->is_host()) {
    } else {
        assert(world->get_player(username_) != nullptr);
        world->get_player(username_)->on_join();
    }
}

ConnectEvent::ConnectEvent(std::string username) : username_(username) {}
ConnectEvent::~ConnectEvent() {}

std::string ConnectEvent::make_packet() const {
    std::string packet = "ConnectEvent " + username_;
    return packet;
}
bool ConnectEvent::reliable() const {
    return true;
};
void ConnectEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    if (network->is_host()) {
        world->load_player(username_, shader);
        world->get_player(username_)->on_join();
        SyncEvent sync {world};
        IAmHostEvent server_connect (receiving_user);
        WeatherUpdateEvent weather_update (world->get_weather()->get_weather_id());
        network->send_packet(sync.make_packet(), sync.reliable(), username_);
        network->send_packet(weather_update.make_packet(), weather_update.reliable(), username_);
        network->send_packet_excluding(make_packet(),reliable(),username_);
        network->send_packet(server_connect.make_packet(),server_connect.reliable(),username_);
    } else {
        world->load_player(username_, shader);
        world->get_player(username_)->on_join();
    }
}

DisconnectEvent::DisconnectEvent(std::string username) : username_(username) {}
DisconnectEvent::~DisconnectEvent() {}

std::string DisconnectEvent::make_packet() const {
    std::string packet = "DisconnectEvent " + username_;
    return packet;
}
bool DisconnectEvent::reliable() const {
    return true;
};
void DisconnectEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    assert(world->get_player(username_) != nullptr);
    if (network->is_host()) {
        world->get_player(username_)->on_disconnect();
        network->send_packet(make_packet(), reliable());
    } else {
        world->get_player(username_)->on_disconnect();
    }
}

SyncEvent::SyncEvent(std::string packet) {
    world_string_ = get_without_first_word(packet);
}

SyncEvent::SyncEvent(std::shared_ptr<World> world) {
    world_string_ = world->to_string();
}

SyncEvent::~SyncEvent() {};

std::string SyncEvent::make_packet() const {
    return "SyncEvent " + world_string_;
};

bool SyncEvent::reliable() const {
    return true;
};

void SyncEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    world->reset_world();
    world->from_string(world_string_, shader);
};

PlayerMoveEvent::PlayerMoveEvent(std::shared_ptr<Player> player) : username_(""), x_(), y_(), z_(), player_(player) {}
PlayerMoveEvent::PlayerMoveEvent(std::string packet) {
    std::vector<std::string> split = split_string(packet);
    username_ = split[1];
    x_ = std::stof(split[2]);
    y_ = std::stof(split[3]);
    z_ = std::stof(split[4]);
}
PlayerMoveEvent::~PlayerMoveEvent(){};

std::string PlayerMoveEvent::make_packet() const {
    if (username_ == "") {
       return "PlayerMoveEvent " + player_->get_username() + " " + std::to_string(player_->get_position().x) + " " + std::to_string(player_->get_position().y) + " " + std::to_string(player_->get_position().z);
    } else {
        return "PlayerMoveEvent " + username_ + " " + std::to_string(x_) + " " + std::to_string(y_) + " " + std::to_string(z_);
    }
}
bool PlayerMoveEvent::reliable() const {
    return false;
};
void PlayerMoveEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    world->get_player(username_)->set_position(Vector3{x_,y_,z_});
    if (network->is_host()) {
        network->send_packet_excluding(make_packet(), reliable(), username_);
    }
}

ObjectMoveEvent::ObjectMoveEvent(std::map<uint32_t, Vector3> objects, std::string sender) : objects_(std::move(objects)), sender_(sender) {};
ObjectMoveEvent::ObjectMoveEvent(std::string packet){
    std::vector<std::string> split = split_string(packet);
    sender_ = split[1];
    for (int i = 2; i < split.size(); i++) {
        std::vector<std::string> update = split_string(split[i]);
        objects_[std::stoi(update[0])] = Vector3{std::stof(update[1]), std::stof(update[2]), std::stof(update[3])};
    }
}
ObjectMoveEvent::~ObjectMoveEvent() {};

std::string ObjectMoveEvent::make_packet() const {
    std::string result = "ObjectMoveEvent " + sender_ + " ";
    for (const auto& p : objects_)
        result += "(" + std::to_string(p.first) + " " + std::to_string(p.second.x) + " " + std::to_string(p.second.y) + " " + std::to_string(p.second.z) + ")";
    return result;
}
bool ObjectMoveEvent::reliable() const {return false;}

void ObjectMoveEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    for (const auto& p : objects_)
        world->update_object(p.first, p.second);
    if (network->is_host()) {
        network->send_packet_excluding(make_packet(), reliable(), sender_);
    }
}
void ObjectMoveEvent::add(uint32_t id, Vector3 position){
    objects_[id] = position;
}

ObjectRemoveEvent::ObjectRemoveEvent(std::vector<uint32_t> indices, std::string sender) : indices_(std::move(indices)), sender_(sender) {}
ObjectRemoveEvent::ObjectRemoveEvent(std::string packet) {
    std::vector<std::string> split = split_string(packet);
    sender_ = split[1];
    for (int i = 2; i < split.size(); i++)
        add(std::stoi(split[i]));
}
ObjectRemoveEvent::~ObjectRemoveEvent() {}
std::string ObjectRemoveEvent::make_packet() const {
    std::string result = "ObjectRemoveEvent " + sender_;
    for (uint32_t index : indices_)
        result += " " + std::to_string(index);
    return result;
}
bool ObjectRemoveEvent::reliable() const {return true;}

void ObjectRemoveEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    for (uint32_t index : indices_)
        world->remove_object(index);
    if (network->is_host())
        network->send_packet_excluding(make_packet(), reliable(), sender_);
}

void ObjectRemoveEvent::add(uint32_t id) {
    indices_.push_back(id);
}

ObjectLoadEvent::ObjectLoadEvent(std::map<uint32_t, std::shared_ptr<Object3d>> objects, std::string sender) : objects_{std::move(objects)}, sender_(sender) {}
ObjectLoadEvent::ObjectLoadEvent(std::string packet) {
    std::vector<std::string> split = split_string(packet);
    sender_ = split[1];
    for (int i = 2; i < split.size(); i++) {
        std::vector<std::string> a = split_string(split[i]);
        std::string type = get_first_word(a[1]);
        std::shared_ptr<Object3d> object;
        if (type == "Cube") {
            object = std::make_shared<Cube>(a[1]);
        } else if (type == "MoveTool") {
            object = std::make_shared<MoveTool>(a[1]);
        } else if (type=="SunTool") {
            object = std::make_shared<SunTool>(a[1]);
        } else if (type=="RotateTool") {
            object = std::make_shared<RotateTool>(a[1]);
        }
        add((uint32_t) std::stoi(a[0]), std::move(object));
    }
}
ObjectLoadEvent::~ObjectLoadEvent() {}
std::string ObjectLoadEvent::make_packet() const {
    std::string result = "ObjectLoadEvent " + sender_ + " ";
    for (const auto& p : objects_)
        result += "(" + std::to_string(p.first) + " (" + p.second->to_string() + "))";
    return result;
}
bool ObjectLoadEvent::reliable() const {
    return true;
}
void ObjectLoadEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    for (const auto& p : objects_)
        world->load_object(p.second, p.first, shader);
    if (network->is_host())
        network->send_packet_excluding(make_packet(), reliable(), sender_);
}
void ObjectLoadEvent::add(uint32_t id, std::shared_ptr<Object3d> object) {
    assert(objects_.find(id) == objects_.end());
    objects_[id] = object;
}

ItemPickupEvent::ItemPickupEvent(std::shared_ptr<Item> item, std::string player) : item_(std::move(item)), player_(player) {}
ItemPickupEvent::ItemPickupEvent(std::string packet) {
    std::vector<std::string> split = split_string(packet);
    std::string type = get_first_word(split[2]);
    player_ = split[1];
    if (type == "MoveTool") {
        item_ = std::make_shared<MoveTool>(split[2]);
    } else if (type == "SunTool") {
        item_ = std::make_shared<SunTool>(split[2]);
    } else if (type == "RotateTool") {
        item_ = std::make_shared<RotateTool>(split[2]);
    }
}
ItemPickupEvent::~ItemPickupEvent() {}
std::string ItemPickupEvent::make_packet() const {
    std::string result = "ItemPickupEvent " + player_ + " (" + item_->to_string() + ")";
    return result;
}
bool ItemPickupEvent::reliable() const {
    return true;
}
void ItemPickupEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    item_->set_shader(shader);
    world->get_player(player_)->set_item(item_);
    if (network->is_host())
        network->send_packet_excluding(make_packet(), reliable(), player_);
}

ItemDropEvent::ItemDropEvent(const std::shared_ptr<Player>& player) : player_(player->get_username()) {}
ItemDropEvent::ItemDropEvent(std::string packet) {
    std::vector<std::string> split = split_string(packet);
    player_ = split[1];
}
ItemDropEvent::~ItemDropEvent() {}
std::string ItemDropEvent::make_packet() const {
    std::string result = "ItemDropEvent " + player_;
    return result;
}
bool ItemDropEvent::reliable() const {
    return true;
}
void ItemDropEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    world->get_player(player_)->drop_item(event_buffer, camera, world, keybinds, dt);
    if (network->is_host())
        network->send_packet_excluding(make_packet(), reliable(), player_);
}

WeatherUpdateEvent::WeatherUpdateEvent(int id) : weather_id_(id), timestamp_offset_(0) {}
WeatherUpdateEvent::WeatherUpdateEvent(int id, int timestamp_offset) : weather_id_(id), timestamp_offset_(timestamp_offset) {}
WeatherUpdateEvent::WeatherUpdateEvent(std::string packet) {
    std::vector<std::string> split = split_string(packet);
    weather_id_ = std::stoi(split[1]);
    timestamp_offset_ = std::stoi(split[2]);
}
WeatherUpdateEvent::~WeatherUpdateEvent() {}
std::string WeatherUpdateEvent::make_packet() const {
    return "WeatherUpdateEvent " + std::to_string(weather_id_) + " " + std::to_string(timestamp_offset_);
}
bool WeatherUpdateEvent::reliable() const {
    return true;
}
void WeatherUpdateEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    if (!network->is_host()) {
        game.get_world()->get_weather()->set_weather_id(weather_id_);
        game.get_world()->get_weather()->update_sun(current_timestamp+timestamp_offset_);
        game.get_world()->update_sun();
    } else {
        game.get_world()->get_weather()->update_sun(current_timestamp+timestamp_offset_);
        game.get_world()->update_sun();
    }
}