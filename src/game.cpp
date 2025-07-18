#include "game.hpp"

Game::Game(std::string current_user) : current_user_(current_user) {};
void Game::load_world(std::string save_file) {};
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