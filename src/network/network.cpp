#include <assert.h>
#include <string>

#include "enet/enet.h"

#include "logging.hpp"
#include "network/network.hpp"
#include "util/factory.hpp"
#include "event/event.hpp"

Network::Network() {
    mode_ = 0;
    initialized_ = !enet_initialize();
    DEBUG("Enet Initialized?: " + std::to_string(initialized_));
    host_ = nullptr;
    server_ = nullptr;
}

Network::~Network() {
    if (host_ != nullptr)
        enet_host_destroy(host_);
    enet_deinitialize();
}

std::unique_ptr<Event> Network::poll_events() {
    std::unique_ptr<Event> result = nullptr;
    if (mode_ == 0)
        return result;
    ENetEvent event;
    std::vector<std::string> split {};
    int i = 0;
    std::string* username;
    if (enet_host_service(host_,&event,0) > 0) {
        std::string data;
        json j;
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            DEBUG("New connection: " + std::to_string(event.peer->address.host) + ", " + std::to_string(event.peer->address.port));
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            data = std::string((char*)event.packet->data, event.packet->dataLength-1);
            INFO("Packet of length " + std::to_string(event.packet->dataLength) + " containing {" + data + "} received from " + std::to_string(event.peer->address.host));
            j = json::parse(data);
            result = std::move(parse_event(j));
            if (j.at("type") == "IAmHostEvent") {
                username = new std::string(j.at("host_username"));
                players_[j.at("host_username")] = event.peer;
                event.peer->data = (void*) (username);
            } else if (j.at("type") == "ConnectEvent") {
                if (is_host()) {
                    username = new std::string(j.at("username"));
                    players_[j.at("username")] = event.peer;
                    event.peer->data = (void*) (username);
                }
            }
            enet_packet_destroy (event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            username = (std::string*)event.peer->data;
            DEBUG("Disconnection: " + std::to_string(event.peer->address.host) + "," + std::to_string(event.peer->address.port) + ", data: " + *username);
            result = std::make_unique<DisconnectEvent>(*username);
            if (is_host()) {
                players_.erase(*username);
            } else {
                enet_host_destroy(host_);
                mode_ = 0;
            }
            delete (std::string*)event.peer->data;
            break;
        }
    }
    return std::move(result);
}

void Network::send_packet(const json& data, bool reliable) const {
    if (mode_ == 0)
        return;
    std::string data_string = data.dump();
    ENetPacket* packet = enet_packet_create(data_string.c_str(), data_string.size()+1, reliable);
    if (mode_ == 1) {
        for (const auto& pair : players_) {
            assert(pair.second != nullptr);
            enet_peer_send(pair.second, 0, packet);
        }
    } else if (mode_ == 2) {
        assert(server_ != nullptr);
        enet_peer_send(server_, 0, packet);
    }
    INFO("Sent packet with: " + data_string);
}

void Network::send_packet_excluding(const json& data, bool reliable, std::string exclude) const {
    if (mode_ == 0)
        return;
    std::string data_string = data.dump();
    ENetPacket* packet = enet_packet_create(data_string.c_str(), data_string.size()+1, reliable);
    if (mode_ == 1) {
        for (const auto& pair : players_) {
            assert(pair.second != nullptr);
            if (pair.first == exclude)
                continue;
            enet_peer_send(pair.second, 0, packet);
        }
    } else if (mode_ == 2) {
        WARN("Client excluded packets not implemented yet");
        // RelayEvent
    }
    INFO("Sent packet excluding " + exclude + ": " + data_string);
}

void Network::send_packet(const json& data, bool reliable, std::string target_username) const {
    if (mode_ == 0)
        return;
    std::string data_string = data.dump();
    ENetPacket* packet = enet_packet_create(data_string.c_str(), data_string.size()+1, reliable);
    if (mode_ == 1) {
        assert(players_.find(target_username) != players_.end());
        enet_peer_send(players_.at(target_username), 0, packet);
    } else if (mode_ == 2) {
        WARN("Client targetted packets not implemented yet");
        // RelayEvent
    }
    INFO("Sent packet targetting " + target_username + ": " + data_string);
}

bool Network::host_server(std::string ip, std::string port) {
    INFO("Attemping to host server with ip: " + ip + ", port " + port);
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = std::stoi(port);
    host_ = enet_host_create(&address,32,1,0,0);
    if (host_ != nullptr) {
        mode_ = 1;
        DEBUG("Hosting server with address " + std::to_string(address.host) + ", port " + std::to_string(address.port));
        return true;
    }
    WARN("Failed to host server with address " + std::to_string(address.host) + ", port " + std::to_string(address.port));
    return false;
};


bool Network::join_server(std::string ip, std::string port) {
    ENetAddress address;
    enet_address_set_host(&address, ip.c_str());
    address.port = std::stoi(port);
    host_ = enet_host_create(NULL,1,1,0,0);
    server_ = enet_host_connect(host_, &address, 1, 0);    
    if (server_ == nullptr) {
        WARN("Failed to find peer " + std::to_string(address.host) + "," + std::to_string(address.port));
        return false;
    }
    
    ENetEvent event;
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(host_, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        DEBUG("Connection to server " + std::to_string(address.host) + "," + std::to_string(address.port) + " succeeded");
        mode_ = 2;
        return true;
    } else {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(server_);
        WARN("Connection to server " + std::to_string(address.host) + "," + std::to_string(address.port) + " failed");
        return false;
    }
};

bool Network::is_online(std::string username) const {
    return players_.find(username) != players_.end();
}

bool Network::is_host() const {
    return mode_ == 1;
}

void Network::disconnect() {
    if (mode_ == 0)
        return;
    if (!is_host()) {
        if (!server_) {
            enet_host_destroy(host_);
            mode_ = 0;
            return;
        }
        ENetEvent event;
        enet_peer_disconnect (server_, 0);
        while (enet_host_service (host_, &event, 3000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy (event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                enet_host_destroy(host_);
                mode_ = 0;
                return;
            }
        }
        enet_peer_reset(server_);
    } else {
        int disconnects = 0;
        if (players_.size() == 0) {
            enet_host_destroy(host_);
            mode_ = 0;
            return;
        }
        for (const auto& pair : players_) {
            assert(pair.second != nullptr);
            enet_peer_disconnect(pair.second, 0);
        }
        ENetEvent event;
        while (enet_host_service (host_, &event, 3000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy (event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                DEBUG("Disconnection succeeded");
                disconnects++;
                if (disconnects == players_.size()) {
                    enet_host_destroy(host_);
                    mode_ = 0;
                    players_.clear();
                    return;
                }
                break;
            }
        }
        for (const auto& pair : players_) {
            assert(pair.second != nullptr);
            enet_peer_reset(pair.second);
        }
        players_.clear();
    }
    mode_ = 0;
    enet_host_destroy(host_);
}

void Network::delete_server() {
    server_ = nullptr;
}