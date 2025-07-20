#include <iostream>

#include "event.hpp"
#include "game.hpp"

Game::Game() : in_world_(false), current_user_("") {
    spawn_point_ = Vector3{0.0f, 0.0f, 0.0f};
    network_ = std::make_unique<Network>();
};

bool Game::in_world() const {
    return in_world_;
}

void Game::poll_events() {
    std::unique_ptr<Event> event = network_->poll_events();
    if (event == nullptr)
        return;
    event->receive(*this);
}

bool Game::host(std::string current_user, std::string save_file, char* ip, char* port) {
    current_user_ = current_user;
    load_world(save_file);
    bool success = network_->host_server(ip, port);
    if (success) {
        in_world_ = true;
        get_player(current_user)->on_join();
    }
    return success;
}

bool Game::join(std::string current_user, char* ip, char* port) {
    current_user_ = current_user;
    bool success = network_->join_server(ip, port);
    if (success) {
        ConnectEvent event (current_user_);
        network_->send_packet(event.make_packet(), true);
        in_world_ = true;
    }
    return success;
}

void Game::load_world(std::string save_file) {
    load_object(std::make_shared<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,1.0f}, RED));
    // TODO: load all existing players from file
    load_player("darek");
};

void Game::save_world(std::string save_file) {};

void Game::reset_world() {
    objects_.clear();
    players_.clear();
};

void Game::load_object(std::shared_ptr<Object3d> object) {
    objects_.push_back(std::move(object));
}

void Game::load_player(std::string username) {
    if (get_player(username) == nullptr) {
        players_.push_back(std::make_shared<Player>(username, spawn_point_));
    }
}

void Game::load_player(std::string username, Vector3 position, bool online) {
    if (get_player(username) == nullptr) {
        players_.push_back(std::make_shared<Player>(username, position, online));
    }
}

const std::vector<std::shared_ptr<Player>>& Game::get_players() {
    return players_;
}

const std::shared_ptr<Player> Game::get_player(std::string username) {
    for (const auto& player : players_) {
        if (player->get_username() == username)
            return player;
    }
    return nullptr;
}

const std::vector<std::shared_ptr<Object3d>>& Game::get_objects() {
    return objects_;
}

const std::string& Game::get_current_user() {
    return current_user_;
}

void Game::sync_clients() {
    SyncEvent event {objects_, players_};
    send_packet(event.make_packet(), true);
};

void Game::sync_client(std::string target_username) {
    SyncEvent event {objects_, players_};
    network_->send_packet(event.make_packet(), true, target_username);
};

bool Game::is_host() const {
    return network_->is_host();
}

bool Game::is_online(std::string username) const {
    return network_->is_online(username);
}

void Game::send_packet(std::string data, bool reliable) const {
    network_->send_packet(data, reliable);
};

void Game::send_packet_excluding(std::string data, bool reliable, std::string exclude) const {
    network_->send_packet_excluding(data, reliable, exclude);
};

void Game::send_packet(std::string data, bool reliable, std::string target_username) const {
    network_->send_packet(data, reliable, target_username);
};