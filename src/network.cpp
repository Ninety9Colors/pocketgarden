#include <iostream>

#include "enet/enet.h"

#include "network.hpp"

Network::Network() {
    mode_ = 0;
    initialized_ = !enet_initialize();
    std::cout << "ENet Initialized: " << initialized_ << "\n";
}

Network::~Network() {
    if (server_ != nullptr)
        enet_host_destroy(server_.get());
    enet_deinitialize();
}

void Network::poll_events() {
    if (mode_ == 0)
        return;
    ENetEvent event;
    while (enet_host_service(server_.get(),&event,0) > 0) {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            std::cout << "New connection: " << event.peer->address.host << ", " << event.peer->address.port << "\n";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            fprintf (stdout,"A packet of length %u containing %s was received from %s on channel %u.\n",
                    event.packet -> dataLength,
                    event.packet -> data,
                    event.peer -> data,
                    event.channelID);
            enet_packet_destroy (event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Disconnection: " << event.peer->data << "\n"; 
            event.peer -> data = NULL;
        }
    }
}

void Network::host_server(std::string ip, std::string port) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = std::stoi(port);
    server_ = std::unique_ptr<ENetHost>(enet_host_create(&address,32,1,0,0));
    if (server_ != NULL)
        mode_ = 1;
    std::cout << "Hosting server with address " << ip << ", port " << port << ". Connected: " << (server_ != NULL) << "\n";
};


void Network::join_server(std::string ip, std::string port) {
    ENetAddress address;
    enet_address_set_host(&address, ip.c_str());
    address.port = std::stoi(port);
    server_ = std::unique_ptr<ENetHost>(enet_host_create(NULL,1,1,0,0));
    std::unique_ptr<ENetPeer> peer = std::unique_ptr<ENetPeer>(enet_host_connect(server_.get(), &address, 1, 0));    
    if (peer == NULL)
    {
    fprintf (stdout, 
                "No available peers for initiating an ENet connection.\n");
    return;
    }
    
    ENetEvent event;
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (server_.get(), &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        fprintf (stdout,"Connection to some.server.net:1234 succeeded.");
        mode_ = 2;
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset (peer.get());
    
        fprintf (stdout,"Connection to some.server.net:1234 failed.");
    }
    std::cout << "Joining server with address " << ip << ", port " << port << "\n";
};