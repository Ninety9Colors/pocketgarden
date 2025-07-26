#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

class Game;
class Network;
class Player;
class World;
class Vector3;

#include "object3d.hpp"

class Event {
public:
    virtual std::string make_packet() const = 0;
    virtual bool reliable() const = 0;
    virtual void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) = 0;
    virtual ~Event() {};
};

class IAmHostEvent : public Event {
public:
    IAmHostEvent(std::string username);
    ~IAmHostEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) override;
private:
    std::string username_;
};

class ConnectEvent : public Event {
public:
    ConnectEvent(std::string username);
    ~ConnectEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) override;
private:
    std::string username_;
};

class DisconnectEvent : public Event {
public:
    DisconnectEvent(std::string username);
    ~DisconnectEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) override;
private:
    std::string username_;
};

class SyncEvent : public Event {
public:
    SyncEvent(std::string packet);
    SyncEvent(std::shared_ptr<World> world);
    ~SyncEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) override;
private:
    std::string world_string_;
};

class PlayerMoveEvent : public Event {
public:
    PlayerMoveEvent(std::shared_ptr<Player> player);
    PlayerMoveEvent(std::string packet);
    ~PlayerMoveEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) override;
private:
    std::shared_ptr<Player> player_;
    std::string username_;
    float x_;
    float y_;
    float z_;
};

class ObjectMoveEvent : public Event {
public:
    ObjectMoveEvent(std::map<uint32_t, Vector3> updates);
    ObjectMoveEvent(std::string packet);
    ~ObjectMoveEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game) override;
    void update(uint32_t id, Vector3 position);
private:
    std::map<uint32_t, Vector3> updates_;
};