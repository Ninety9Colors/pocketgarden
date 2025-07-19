#include "game.hpp"

Game::Game() : in_world_(false), current_user_("") {};

bool Game::in_world() const {
    return in_world_;
}

void Game::poll_events() {network_.poll_events();}

void Game::host(std::string current_user, std::string save_file, char* ip, char* port) {
    current_user_ = current_user;
    load_world(save_file);
    network_.host_server(ip, port);
    in_world_ = true;
}

void Game::join(std::string current_user, char* ip, char* port) {
    current_user_ = current_user;
    // sync_world();
    network_.join_server(ip, port);
    // in_world_ = true;
}

void Game::load_world(std::string save_file) {
    add_object(std::make_unique<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,1.0f}, RED));
    auto isa = std::make_unique<Player>("isabella", Vector3{1.0f, 0.0f, 0.0f});
    auto darek = std::make_unique<Player>("darek", Vector3{-1.0f, 0.0f, 0.0f});
    add_player(std::move(isa));
    add_player(std::move(darek));
};

void Game::save_world(std::string save_file) {};

void Game::add_object(std::unique_ptr<Object3d> object) {
    objects_.push_back(std::move(object));
}

void Game::add_player(std::shared_ptr<Player> player) {
    players_.push_back(std::move(player));
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

const std::vector<std::unique_ptr<Object3d>>& Game::get_objects() {
    return objects_;
}

const std::string& Game::get_current_user() {
    return current_user_;
}