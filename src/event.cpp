#include <assert.h>
#include <memory>

#include "event.hpp"
#include "game.hpp"
#include "player.hpp"

ConnectEvent::ConnectEvent(std::string username) : username_(username) {}
ConnectEvent::~ConnectEvent() {}

std::string ConnectEvent::make_packet() {
    std::string packet = "ConnectEvent " + username_;
    return packet;
}
void ConnectEvent::receive(Game& game) {
    game.load_player(username_);
    game.get_player(username_)->on_join();
    game.sync_clients();
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
    std::vector<std::string> split {};
    int l = 0;
    int r = 0;
    while (r < object_packet_string_.size()) {
        while (r < object_packet_string_.size() && object_packet_string_[r] != ' ')
            r++;
        if (r >= object_packet_string_.size())
            break;
        split.push_back(object_packet_string_.substr(l, r-l));
        l=r+1;
        r++;
    }
    split.push_back(object_packet_string_.substr(l, r-l));
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
    split.clear();
    l = 0;
    r = 0;
    while (r < player_packet_string_.size()) {
        while (r < player_packet_string_.size() && player_packet_string_[r] != ' ')
            r++;
        if (r >= player_packet_string_.size())
            break;
        split.push_back(player_packet_string_.substr(l, r-l));
        l=r+1;
        r++;
    }
    split.push_back(player_packet_string_.substr(l, r-l));
    i = 0;
    while (i < split.size()) {
        if (split[i] == "Player") {
            std::string username = split[i+1];
            Vector3 position {std::stof(split[i+2]), std::stof(split[i+3]), std::stof(split[i+4])};
            game.load_player(username, position);
            i = i+5;
        }
    }
    for (const auto player : game.get_players()) {
        // if (player->get_username() == game.get_current_user())
        //     continue;
        // if (game.is_online(player->get_username()))
        //     player->on_join();
        player->on_join();
    }
};