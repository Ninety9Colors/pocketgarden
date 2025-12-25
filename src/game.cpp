#include <iostream>

#include "event/event.hpp"
#include "game.hpp"
#include "settings.hpp"
#include "logging.hpp"
#include <cassert>

Game::Game() : world_(), network_(), main_camera_(), event_buffer_receive_(), event_buffer_send_(), in_world_(false), current_username_("") {
};

bool Game::in_world() const {
    return in_world_;
}

bool Game::host(std::string current_user, std::string save_file, char* ip, char* port) {
    INFO("Game.host called, attempting to start world:");
    current_username_ = current_user;
    world_.load_world(save_file);
    INFO(" - Loaded world");
    world_.load_player(current_user);
    INFO(" - Loaded current user");
    bool success = network_.host_server(ip, port);
    if (success) {
        INFO("Successfully hosted with ip: " + std::string(ip) + ", and port: " + std::string(port));
        in_world_ = true;
        world_.disconnect_players();
        world_.get_player(current_user)->get().set_online(true);
        event_buffer_receive_.clear();
        event_buffer_send_.clear();
    } else {
        INFO("Failed to host with ip: " + std::string(ip) + ", and port: " + std::string(port));
    }
    return success;
}

bool Game::join(std::string current_user, char* ip, char* port) {
    current_username_ = current_user;
    bool success = network_.join_server(ip, port);
    if (success) {
        INFO("Successfully connected to ip: " + std::string(ip) + ", and port: " + std::string(port));
        ConnectEvent event (current_user);
        network_.send_packet(event.to_json(), event.reliable());
        in_world_ = true;
        event_buffer_receive_.clear();
        event_buffer_send_.clear();
    }
    return success;
}

const std::string& Game::get_current_username() const {
    return current_username_;
}

std::optional<std::reference_wrapper<Player>> Game::get_current_player() {
    return world_.get_player(current_username_);
}

void Game::poll_events() {
    auto event = network_.poll_events();
    if (event == nullptr)
        return;
    event_buffer_receive_.push_back(std::move(event));
}

void Game::tick(const std::vector<bool>& keybinds, uint64_t current_timestamp, float dt) {
    while (!event_buffer_receive_.empty()) {
        INFO("Tick receiving event: " + std::string(event_buffer_receive_.front()->to_json()["type"]));
        event_buffer_receive_.front()->receive(*this,current_timestamp,keybinds,dt,network_.is_host());
        event_buffer_receive_.pop_front();
    }
    while (!event_buffer_send_.empty()) {
        INFO("Tick sending event: " + std::string(event_buffer_send_.front()->to_json()["type"]));
        network_.send_packet(event_buffer_send_.front()->to_json(),event_buffer_send_.front()->reliable());
        event_buffer_send_.pop_front();
    }
}

void Game::update_current_player(const std::vector<bool>& keybinds, float dt) {
    // Player Movement
    if (main_camera_.get_mode() != CAMERA_CUSTOM) return;
    auto current_player = world_.get_player(current_username_);
    if (!current_player) {
        CRITICAL("Trying to update current player that does not exist!");
        return;
    }
    if (current_player->get().is_online() && (keybinds[0] || keybinds[1] || keybinds[2] || keybinds[3])) {
        current_player->get().move(main_camera_.get_direction(),keybinds,dt);
        event_buffer_send_.push_back(std::make_unique<PlayerMoveEvent>(current_username_,current_player->get().get_position()));
    }
    // Item Usage
    if (!keybinds[9]) {
        current_player->get().use_item(*this, keybinds, dt);
        return;
    }
    // Item Pickups
    Ray ray = Ray(main_camera_.get_position(),main_camera_.get_direction());
    uint32_t pickup_id = world_.raycast_nearest(ray,current_player->get().get_pickup_range(),static_cast<uint8_t>(ObjectType::ITEM));
    std::unique_ptr<Object3d> dropped = current_player->get().drop_item(*this,current_username_,keybinds,dt);
    if (dropped != nullptr) {
        json j = dropped->to_json();
        uint32_t id = world_.load_object(std::move(dropped));
        event_buffer_send_.push_back(std::make_unique<ItemDropEvent>(current_username_));
        event_buffer_send_.push_back(std::make_unique<ObjectLoadEvent>(id,j,current_username_));
    }
    if (pickup_id != 0) {
        std::unique_ptr<Object3d> picked_up = std::move(world_.transfer_object(pickup_id));
        json j = picked_up->to_json();
        current_player->get().set_item(std::move(picked_up));
        INFO("picked up item with id: " + std::to_string(pickup_id));
        event_buffer_send_.push_back(std::make_unique<ItemPickupEvent>(j,current_username_));
        event_buffer_send_.push_back(std::make_unique<ObjectRemoveEvent>(pickup_id,current_username_));
    }
}

void Game::update_main_camera(Vector2 mouse_delta) {
    main_camera_.update(world_.get_player(current_username_)->get().get_position(),mouse_delta);
}

void Game::queue_event_send(std::unique_ptr<Event> event) {
    event_buffer_send_.push_back(std::move(event));
}
void Game::queue_event_receive(std::unique_ptr<Event> event) {
    event_buffer_receive_.push_back(std::move(event));
}

void Game::close_game() {
    if (!in_world_) return;
    if (network_.is_host())
        world_.save_world("test world.json");
    network_.disconnect();
    in_world_ = false;
    EnableCursor();
}

const Network& Game::get_network() const {
    return network_;
};

MainCamera& Game::get_camera() {
    return main_camera_;
}

World& Game::get_world() {
    return world_;
};