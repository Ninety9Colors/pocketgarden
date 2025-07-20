#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "event.hpp"

struct _ENetHost;
typedef struct _ENetHost ENetHost;
struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

class Network {
public:
    Network();
    ~Network();

    std::unique_ptr<Event> poll_events();
    void send_packet(std::string data, bool reliable) const;
    void send_packet_excluding(std::string data, bool reliable, std::string exclude) const;
    void send_packet(std::string data, bool reliable, std::string target_username) const;
    bool host_server(std::string ip, std::string port);
    bool join_server(std::string ip, std::string port);
    bool is_online(std::string username) const;
    bool is_host() const;
private:
    bool initialized_;
    int mode_; // 0 - none, 1 - host, 2 - join
    ENetHost* host_;
    ENetPeer* server_;
    std::map<std::string, ENetPeer*> players_;
};