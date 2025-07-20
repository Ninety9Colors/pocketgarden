#pragma once
#include <memory>
#include <string>
#include <vector>

class Game;
class Player;

#include "object3d.hpp"

class Event {
public:
    virtual std::string make_packet() = 0;
    virtual void receive(Game& game) = 0;
    virtual ~Event() {};
};

class IAmHostEvent : public Event {
public:
    IAmHostEvent(std::string username);
    ~IAmHostEvent();
    std::string make_packet() override;
    void receive(Game& game) override;
private:
    std::string username_;
};

class ConnectEvent : public Event {
public:
    ConnectEvent(std::string username);
    ~ConnectEvent();
    std::string make_packet() override;
    void receive(Game& game) override;
private:
    std::string username_;
};

class DisconnectEvent : public Event {
public:
    DisconnectEvent(std::string username);
    ~DisconnectEvent();
    std::string make_packet() override;
    void receive(Game& game) override;
private:
    std::string username_;
};

class SyncEvent : public Event {
public:
    SyncEvent(std::string packet);
    SyncEvent(const std::vector<std::shared_ptr<Object3d>>& objects, const std::vector<std::shared_ptr<Player>>& players);
    ~SyncEvent();
    std::string make_packet() override;
    void receive(Game& game) override;
private:
    std::string object_packet_string_;
    std::string player_packet_string_;
};

class PlayerMoveEvent : public Event {
public:
    PlayerMoveEvent(std::string username, float x, float y, float z);
    PlayerMoveEvent(std::string packet);
    ~PlayerMoveEvent();
    std::string make_packet() override;
    void receive(Game& game) override;
private:
    std::string username_;
    float x_;
    float y_;
    float z_;
};