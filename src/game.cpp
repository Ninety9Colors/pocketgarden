#include <iostream>

#include "event.hpp"
#include "game.hpp"

Game::Game() : in_world_(false), current_user_("") {
    world_ = std::make_shared<World>();
    network_ = std::make_unique<Network>();
};

bool Game::in_world() const {
    return in_world_;
}

void Game::poll_events(std::string receiving_user, std::shared_ptr<World> world, std::shared_ptr<Network> network, Game& game, uint64_t current_timestamp, std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, const std::vector<bool>& keybinds, float dt, std::shared_ptr<Shader> shader) {
    std::unique_ptr<Event> event = network_->poll_events();
    if (event == nullptr)
        return;
    event->receive(receiving_user,world,network,game,current_timestamp,event_buffer,camera,keybinds,dt,shader);
}

bool Game::host(std::string current_user, std::string save_file, char* ip, char* port, std::shared_ptr<Shader> shader) {
    current_user_ = current_user;
    world_->load_world(save_file, shader);
    world_->load_player(current_user, shader);
    world_->set_alone(current_user);
    bool success = network_->host_server(ip, port);
    if (success) {
        in_world_ = true;
        world_->get_player(current_user)->on_join();
    }
    return success;
}

bool Game::join(std::string current_user, char* ip, char* port) {
    current_user_ = current_user;
    bool success = network_->join_server(ip, port);
    if (success) {
        ConnectEvent event (current_user_);
        network_->send_packet(event.make_packet(), event.reliable());
        in_world_ = true;
    }
    return success;
}

const std::string& Game::get_current_user() {
    return current_user_;
}

const std::shared_ptr<Player> Game::get_current_player() {
    return world_->get_player(current_user_);
}

void Game::disconnect() {
    if (network_->is_host()) {
        get_world()->save_world("test world.data");
    }
    network_->disconnect();
    in_world_ = false;
    EnableCursor();
}

std::shared_ptr<Network> Game::get_network() const {
    return network_;
};

std::shared_ptr<World> Game::get_world() const {
    return world_;
};