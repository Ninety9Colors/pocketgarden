#include <assert.h>
#include <memory>

#include "event.hpp"
#include "game.hpp"
#include "player.hpp"
#include "util.hpp"

IAmHostEvent::IAmHostEvent(std::string username) : username_(username) {}
IAmHostEvent::~IAmHostEvent() {}

std::string IAmHostEvent::make_packet() {
    std::string packet = "IAmHostEvent " + username_;
    return packet;
}
void IAmHostEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    if (network->is_host()) {
    } else {
        assert(world->get_player(username_) != nullptr);
        world->get_player(username_)->on_join();
    }
}

ConnectEvent::ConnectEvent(std::string username) : username_(username) {}
ConnectEvent::~ConnectEvent() {}

std::string ConnectEvent::make_packet() {
    std::string packet = "ConnectEvent " + username_;
    return packet;
}
void ConnectEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    if (network->is_host()) {
        world->load_player(username_);
        world->get_player(username_)->on_join();
        SyncEvent sync {world};
        ConnectEvent relay (username_);
        IAmHostEvent server_connect (receiving_user);
        network->send_packet(sync.make_packet(), true, username_);
        network->send_packet_excluding(relay.make_packet(),true,username_);
        network->send_packet(server_connect.make_packet(),true,username_);
    } else {
        world->load_player(username_);
        world->get_player(username_)->on_join();
    }
}

DisconnectEvent::DisconnectEvent(std::string username) : username_(username) {}
DisconnectEvent::~DisconnectEvent() {}

std::string DisconnectEvent::make_packet() {
    std::string packet = "DisconnectEvent " + username_;
    return packet;
}
void DisconnectEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    assert(world->get_player(username_) != nullptr);
    if (network->is_host()) {
        world->get_player(username_)->on_disconnect();
        DisconnectEvent disconnect_event (username_);
        network->send_packet(disconnect_event.make_packet(), true);
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

std::string SyncEvent::make_packet() {
    return "SyncEvent " + world_string_;
};

void SyncEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    world->reset_world();
    world->from_string(world_string_);
};

PlayerMoveEvent::PlayerMoveEvent(std::string username, float x, float y, float z) : username_(username), x_(x), y_(y), z_(z) {}
PlayerMoveEvent::PlayerMoveEvent(std::string packet) {
    std::vector<std::string> split = split_string(packet);
    username_ = split[1];
    x_ = std::stof(split[2]);
    y_ = std::stof(split[3]);
    z_ = std::stof(split[4]);
}
PlayerMoveEvent::~PlayerMoveEvent(){};
std::string PlayerMoveEvent::make_packet() {
    return "PlayerMoveEvent " + username_ + " " + std::to_string(x_) + " " + std::to_string(y_) + " " + std::to_string(z_);
}
void PlayerMoveEvent::receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) {
    world->get_player(username_)->set_position(x_,y_,z_);
    if (network->is_host()) {
        PlayerMoveEvent relay (username_,x_,y_,z_);
        network->send_packet_excluding(relay.make_packet(), false, username_);
    }
}