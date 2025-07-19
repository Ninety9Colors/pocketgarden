#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>

struct _ENetHost;
typedef struct _ENetHost ENetHost;
struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

class Network {
public:
    Network();
    ~Network();

    void poll_events();
    void host_server(std::string ip, std::string port);
    void join_server(std::string ip, std::string port);
private:
    bool initialized_;
    int mode_; // 0 - none, 1 - host, 2 - join
    std::unique_ptr<ENetHost> server_;
    std::map<std::string, std::shared_ptr<ENetPeer>> players_;
};