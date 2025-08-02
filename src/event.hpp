#pragma once
#include <map>
#include <memory>
#include <cstdint>
#include <string>
#include <vector>

class Game;
class Network;
class Player;
class World;
class Vector3;
class MainCamera;

#include "object3d.hpp"

class Event {
public:
    virtual std::string make_packet() const = 0;
    virtual bool reliable() const = 0;
    virtual void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) = 0;
    virtual ~Event() {};
};

class IAmHostEvent : public Event {
public:
    IAmHostEvent(std::string username);
    ~IAmHostEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
private:
    std::string username_;
};

class ConnectEvent : public Event {
public:
    ConnectEvent(std::string username);
    ~ConnectEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
private:
    std::string username_;
};

class DisconnectEvent : public Event {
public:
    DisconnectEvent(std::string username);
    ~DisconnectEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
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
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
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
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
private:
    std::shared_ptr<Player> player_;
    std::string username_;
    float x_;
    float y_;
    float z_;
};

class ObjectMoveEvent : public Event {
public:
    ObjectMoveEvent(std::map<uint32_t, Vector3> objects, std::string sender);
    ObjectMoveEvent(std::string packet);
    ~ObjectMoveEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
    void add(uint32_t id, Vector3 position);
private:
    std::map<uint32_t, Vector3> objects_;
    std::string sender_;
};

class ObjectRemoveEvent : public Event {
public:
    ObjectRemoveEvent(std::vector<uint32_t> indices, std::string sender);
    ObjectRemoveEvent(std::string packet);
    ~ObjectRemoveEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
    void add(uint32_t id);
private:
    std::vector<uint32_t> indices_;
    std::string sender_;
};

class ObjectLoadEvent : public Event {
public:
    ObjectLoadEvent(std::map<uint32_t, std::shared_ptr<Object3d>> objects, std::string sender);
    ObjectLoadEvent(std::string packet);
    ~ObjectLoadEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
    void add(uint32_t id, std::shared_ptr<Object3d> object);
private:
    std::map<uint32_t, std::shared_ptr<Object3d>> objects_;
    std::string sender_;
};

class ItemPickupEvent : public Event {
public:
    ItemPickupEvent(std::shared_ptr<Item> item, std::string player);
    ItemPickupEvent(std::string packet);
    ~ItemPickupEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
private:
    std::shared_ptr<Item> item_;
    std::string player_;
};

class ItemDropEvent : public Event {
public:
    ItemDropEvent(const std::shared_ptr<Player>& player);
    ItemDropEvent(std::string packet);
    ~ItemDropEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
private:
    std::string player_;
};

class WeatherUpdateEvent : public Event {
public:
    WeatherUpdateEvent(int id);
    WeatherUpdateEvent(int id, int timestamp_offset);
    WeatherUpdateEvent(std::string packet);
    ~WeatherUpdateEvent();
    std::string make_packet() const override;
    bool reliable() const override;
    void receive(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) override;
private:
    int weather_id_;
    int timestamp_offset_;
};