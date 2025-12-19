#include <iostream>

#include "event/event.hpp"
#include "game.hpp"
#include "settings.hpp"
#include "logging.hpp"
#include <cassert>

Game::Game() : event_buffer_(), main_camera_(), world_(), network_(), (false), current_username_("") {
    // TODO: Load settings from file
    INFO("Initializing Settings");
    Settings::set("Camera Sensitivity",0.001f);
};

bool Game::in_world() const {
    return in_world_;
}

bool Game::host(std::string current_user, std::string save_file, char* ip, char* port) {
    current_username_ = current_user;
    world_.load_world(save_file);
    world_.load_player(current_user);
    bool success = network_.host_server(ip, port);
    if (success) {
        in_world_ = true;
        world_.get_player(current_user)->get().set_online(true);
    }
    return success;
}

bool Game::join(std::string current_user, char* ip, char* port) {
    current_username_ = current_user;
    bool success = network_.join_server(ip, port);
    if (success) {
        ConnectEvent event (current_user_);
        network_->send_packet(event.make_packet(), event.reliable());
        in_world_ = true;
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
    event_buffer_.push_back(event);
}

void Game::tick(uint64_t current_timestamp) {
    auto player = world_.get_player(current_username_);
    if (!player) {
        CRITICAL("Could not find current player, but trying to run a game tick!");
        return;
    }
    while (!event_buffer_.empty()) {
        event_buffer_.front()->receive(game,current_timestamp);
        network_.send_packet(event_buffer_.front()->make_packet(),event_buffer_.front()->reliable());
        event_buffer_.pop_front();
    }
}

void Game::update_current_player() {
    bool moved = move(camera, keybinds, dt);
    uint32_t pickup_id = try_pickup(camera, world, keybinds);
    use_item(event_buffer, camera, world, keybinds, dt);
    if (moved) {
        event_buffer["PlayerMoveEvent"] = std::make_shared<PlayerMoveEvent>(shared_from_this());
    }
    if (pickup_id != 0) {
        std::shared_ptr<Item> dropped = drop_item(event_buffer,camera,world,keybinds,dt);
        if (dropped != nullptr) {
            uint32_t id = world->load_object(dropped, dropped->get_shader());
            event_buffer["ItemDropEvent"] = std::make_shared<ItemDropEvent>(shared_from_this());
            if (event_buffer.find("ObjectLoadEvent") == event_buffer.end()) {
                std::shared_ptr<ObjectLoadEvent> load_event = std::make_shared<ObjectLoadEvent>(std::map<uint32_t,std::shared_ptr<Object3d>>{}, get_username());
                load_event->add(id, dropped);
                event_buffer["ObjectLoadEvent"] = load_event;
            } else {
                std::dynamic_pointer_cast<ObjectLoadEvent>(event_buffer["ObjectLoadEvent"])->add(id, dropped);
            }
        }
        std::shared_ptr<Item> item = std::dynamic_pointer_cast<Item>(world->get_objects().at(pickup_id));
        set_item(item);
        world->remove_object(pickup_id);
        event_buffer["ItemPickupEvent"] = std::make_shared<ItemPickupEvent>(item, get_username());
        if (event_buffer.find("ObjectRemoveEvent") == event_buffer.end()) {
            std::shared_ptr<ObjectRemoveEvent> remove_event = std::make_shared<ObjectRemoveEvent>(std::vector<uint32_t>{}, get_username());
            remove_event->add(pickup_id);
            event_buffer["ObjectRemoveEvent"] = std::move(remove_event);
        } else {
            std::dynamic_pointer_cast<ObjectRemoveEvent>(event_buffer["ObjectRemoveEvent"])->add(pickup_id);
        }
    } else if (keybinds[9]) {
        std::shared_ptr<Item> dropped = drop_item(event_buffer,camera,world,keybinds,dt);
        if (dropped == nullptr)
            return;
        event_buffer["ItemDropEvent"] = std::make_shared<ItemDropEvent>(shared_from_this());
        uint32_t id = world->load_object(dropped, dropped->get_shader());
        if (event_buffer.find("ObjectLoadEvent") == event_buffer.end()) {
            std::shared_ptr<ObjectLoadEvent> load_event = std::make_shared<ObjectLoadEvent>(std::map<uint32_t,std::shared_ptr<Object3d>>{}, get_username());
            load_event->add(id, dropped);
            event_buffer["ObjectLoadEvent"] = load_event;
        } else {
            std::dynamic_pointer_cast<ObjectLoadEvent>(event_buffer["ObjectLoadEvent"])->add(id, dropped);
        }
    }
}

void Game::close_game() {
    if (network_.is_host())
        world_.save_world("test world.data");
    network_.disconnect();
    in_world_ = false;
    EnableCursor();
}

const Network& Game::get_network() const {
    return network_;
};

const World& Game::get_world() const {
    return world_;
};