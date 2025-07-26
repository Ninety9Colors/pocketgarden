#include <assert.h>
#include <memory>

#include "raylib.h"

#include "event.hpp"
#include "game.hpp"
#include "player.hpp"
#include "util.hpp"

IAmHostEvent::IAmHostEvent(std::string username) : username_(username) {}
IAmHostEvent::~IAmHostEvent() {}

std::string IAmHostEvent::make_packet() const {
    std::string packet = "IAmHostEvent " + username_;
    return packet;
}

bool IAmHostEvent::reliable() const {
    return true;
};

void IAmHostEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
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
void ConnectEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    if (network->is_host()) {
        world->load_player(username_);
        world->get_player(username_)->on_join();
        SyncEvent sync {world};
        ConnectEvent relay (username_);
        IAmHostEvent server_connect (receiving_user);
        network->send_packet(sync.make_packet(), sync.reliable(), username_);
        network->send_packet_excluding(relay.make_packet(),relay.reliable(),username_);
        network->send_packet(server_connect.make_packet(),server_connect.reliable(),username_);
    } else {
        world->load_player(username_);
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
void DisconnectEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    assert(world->get_player(username_) != nullptr);
    if (network->is_host()) {
        world->get_player(username_)->on_disconnect();
        DisconnectEvent disconnect_event (username_);
        network->send_packet(disconnect_event.make_packet(), disconnect_event.reliable());
    } else {
        network->delete_server();
        game.disconnect();
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

void SyncEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    world->reset_world();
    world->from_string(world_string_);
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
void PlayerMoveEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    world->get_player(username_)->set_position(x_,y_,z_);
    if (network->is_host()) {
        network->send_packet_excluding(make_packet(), reliable(), username_);
    }
}

ObjectMoveEvent::ObjectMoveEvent(std::map<uint32_t, Vector3> updates) : updates_(std::move(updates)) {};
ObjectMoveEvent::ObjectMoveEvent(std::string packet){
    std::vector<std::string> split = split_string(packet);
    for (int i = 1; i < split.size(); i++) {
        std::vector<std::string> update = split_string(split[i]);
        updates_[std::stoi(update[0])] = Vector3{std::stof(update[1]), std::stof(update[2]), std::stof(update[3])};
    }
}
ObjectMoveEvent::~ObjectMoveEvent() {};

std::string ObjectMoveEvent::make_packet() const {
    std::string result = "ObjectMoveEvent ";
    for (const auto& p : updates_)
        result += "(" + std::to_string(p.first) + " " + std::to_string(p.second.x) + " " + std::to_string(p.second.y) + " " + std::to_string(p.second.z) + ")";
    return result;
}
bool ObjectMoveEvent::reliable() const {return false;}

void ObjectMoveEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    for (const auto& p : updates_)
        world->update_object(p.first, p.second);
}
void ObjectMoveEvent::update(uint32_t id, Vector3 position){
    updates_[id] = position;
}