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
void IAmHostEvent::receive(Game& game) {
    if (game.is_host()) {
    } else {
        game.load_player(username_);
        game.get_player(username_)->on_join();
    }
}

ConnectEvent::ConnectEvent(std::string username) : username_(username) {}
ConnectEvent::~ConnectEvent() {}

std::string ConnectEvent::make_packet() {
    std::string packet = "ConnectEvent " + username_;
    return packet;
}
void ConnectEvent::receive(Game& game) {
    if (game.is_host()) {
        game.load_player(username_);
        game.get_player(username_)->on_join();
        game.sync_client(username_);
        ConnectEvent relay (username_);
        IAmHostEvent server_connect (game.get_current_user());
        game.send_packet_excluding(relay.make_packet(),true,username_);
        game.send_packet(server_connect.make_packet(),true,username_);
    } else {
        game.load_player(username_);
        game.get_player(username_)->on_join();
    }
}

DisconnectEvent::DisconnectEvent(std::string username) : username_(username) {}
DisconnectEvent::~DisconnectEvent() {}

std::string DisconnectEvent::make_packet() {
    std::string packet = "DisconnectEvent " + username_;
    return packet;
}
void DisconnectEvent::receive(Game& game) {
    assert(game.get_player(username_) != nullptr);
    if (game.is_host()) {
        game.get_player(username_)->on_disconnect();
        DisconnectEvent disconnect_event (username_);
        game.send_packet(disconnect_event.make_packet(), true);
    } else {
        game.get_player(username_)->on_disconnect();
    }
}

SyncEvent::SyncEvent(std::string packet) {
    int l,r = 0;
    while (r < packet.size()) {
        while (r < packet.size() && packet[r] != ' ')
            r++;
        if (r >= packet.size())
            break;
        std::string sub = packet.substr(l, r-l);
        if (sub == "Cube") {
            bool add_space = object_packet_string_.size() != 0;
            if (add_space)
                object_packet_string_ += " ";
            object_packet_string_ += "Cube ";
            l=r+1;
            r++;
            while (r < packet.size() && packet[r] != 'C' && packet[r] != 'P') {
                r++;
            }
            if (r != packet.size()) r--;
            object_packet_string_ += packet.substr(l,r-l);
        } else if (sub == "Player") {
            bool add_space = player_packet_string_.size() != 0;
            if (add_space)
                player_packet_string_ += " ";
            player_packet_string_ += "Player ";
            l=r+1;
            r++;
            while (r < packet.size() && packet[r] != 'C' && packet[r] != 'P') {
                r++;
            }
            if (r != packet.size()) r--;
            player_packet_string_ += packet.substr(l,r-l);
        }
        l=r+1;
        r++;
    }
}

SyncEvent::SyncEvent(const std::vector<std::shared_ptr<Object3d>>& objects, const std::vector<std::shared_ptr<Player>>& players) {
    for (int i = 0; i < objects.size(); i++) {
        const auto& object = objects[i];
        object_packet_string_ += object->get_packet_string();
        if (i != objects.size()-1)
            object_packet_string_ += " ";
    }
    for (int i = 0; i < players.size(); i++) {
        const auto& player = players[i];
        player_packet_string_ += player->get_packet_string();
        if (i != players.size()-1)
            player_packet_string_ += " ";
    }
};
SyncEvent::~SyncEvent() {};
std::string SyncEvent::make_packet() {
    return "SyncEvent " + object_packet_string_ + " " + player_packet_string_;
};

void SyncEvent::receive(Game& game) {
    game.reset_world();
    std::vector<std::string> split = split_string(object_packet_string_);
    int i = 0;
    while (i < split.size()) {
        if (split[i] == "Cube") {
            Vector3 position {std::stof(split[i+1]), std::stof(split[i+2]), std::stof(split[i+3])};
            Vector3 size {std::stof(split[i+4]), std::stof(split[i+5]), std::stof(split[i+6])};
            Color color {std::stoi(split[i+7]), std::stoi(split[i+8]), std::stoi(split[i+9]), std::stoi(split[i+10])};
            game.load_object(std::make_shared<Cube>(position,size,color));
            i = i+11;
        }
    }
    split = split_string(player_packet_string_);
    i = 0;
    while (i < split.size()) {
        if (split[i] == "Player") {
            std::string username = split[i+1];
            Vector3 position {std::stof(split[i+2]), std::stof(split[i+3]), std::stof(split[i+4])};
            bool online = std::stoi(split[i+5]) == 1;
            game.load_player(username, position, online);
            i = i+6;
        }
    }
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
void PlayerMoveEvent::receive(Game& game) {
    game.get_player(username_)->set_position(x_,y_,z_);
    if (game.is_host()) {
        PlayerMoveEvent relay (username_,x_,y_,z_);
        game.send_packet_excluding(relay.make_packet(), false, username_);
    }
}