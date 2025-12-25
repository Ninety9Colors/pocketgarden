#pragma once
#include <map>
#include <memory>
#include <cstdint>
#include <string>
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

class Game;
class Network;
class Player;
class World;
class Vector2;
class Vector3;
class Vector4;
typedef Vector4 Quaternion;
class MainCamera;
class Object3d;
class Item;
class Shader;

class Event {
public:
    Event();
    virtual ~Event() {};

    virtual json to_json() const = 0;
    virtual void from_json(const json& j) = 0;

    virtual bool reliable() const = 0;
    virtual void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) = 0;
};

class IAmHostEvent : public Event {
public:
    IAmHostEvent(std::string host_username);
    IAmHostEvent(const json& j);
    ~IAmHostEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;

    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    std::string host_username_;
};

class ConnectEvent : public Event {
public:
    ConnectEvent(std::string username);
    ConnectEvent(const json& j);
    ~ConnectEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    std::string username_;
};

class DisconnectEvent : public Event {
public:
    DisconnectEvent(std::string username);
    DisconnectEvent(const json& j);
    ~DisconnectEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    std::string username_;
};

class SyncEvent : public Event {
public:
    SyncEvent(const World& world);
    SyncEvent(const json& j);
    ~SyncEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    json world_json_;
};

class PlayerMoveEvent : public Event {
public:
    PlayerMoveEvent(std::string username, Vector3 position);
    PlayerMoveEvent(const json& j);
    ~PlayerMoveEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    std::unique_ptr<Vector3> position_;
    std::string username_;
};

class ObjectMoveEvent : public Event {
public:
    ObjectMoveEvent(uint32_t id, Vector3 position, std::string sender);
    ObjectMoveEvent(const json& j);
    ~ObjectMoveEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    uint32_t id_;
    std::unique_ptr<Vector3> position_;
    std::string sender_;
};

class ObjectRotateEvent : public Event {
public:
    ObjectRotateEvent(uint32_t id, Quaternion quaternion, std::string sender);
    ObjectRotateEvent(const json& j);
    ~ObjectRotateEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    uint32_t id_;
    std::unique_ptr<Quaternion> quaternion_;
    std::string sender_;
};

class ObjectRemoveEvent : public Event {
public:
    ObjectRemoveEvent(uint32_t id, std::string sender);
    ObjectRemoveEvent(const json& j);
    ~ObjectRemoveEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    uint32_t id_;
    std::string sender_;
};

class ObjectLoadEvent : public Event {
public:
    ObjectLoadEvent(uint32_t id, json object_json, std::string sender);
    ObjectLoadEvent(const json& j);
    ~ObjectLoadEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    uint32_t id_;
    json object_json_;
    std::string sender_;
};

class ItemPickupEvent : public Event {
public:
    ItemPickupEvent(json item_json, std::string username);
    ItemPickupEvent(const json& j);
    ~ItemPickupEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    json item_json_;
    std::string username_;
};

class ItemDropEvent : public Event {
public:
    ItemDropEvent(std::string username);
    ItemDropEvent(const json& j);
    ~ItemDropEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    std::string username_;
};

class WeatherUpdateEvent : public Event {
public:
    WeatherUpdateEvent(int weather_id, int timestamp_offset=0);
    WeatherUpdateEvent(const json& j);
    ~WeatherUpdateEvent();
    json to_json() const override;
    void from_json(const json& j) override;
    bool reliable() const override;
    void receive(Game& game, uint64_t current_timestamp, const std::vector<bool>& keybinds, float dt, bool is_host) override;
private:
    int weather_id_;
    int timestamp_offset_;
};