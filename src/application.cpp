#include <memory>

#include "raylib.h"

#include "application.hpp"
#include "cube.hpp"
#include "maincamera.hpp"

Application::Application() {
    InitWindow(1920, 1280, "PocketLife");
}

void Application::run(Game& game) {
    game.add_object(std::make_unique<Cube>(Vector3{0.0f,0.0f,0.0f}, Vector3{1.0f,1.0f,1.0f}, RED));
    auto isa = std::make_unique<Player>("isabella", Vector3{1.0f, 0.0f, 0.0f});
    auto darek = std::make_unique<Player>("darek", Vector3{-1.0f, 0.0f, 0.0f});
    isa->on_join();
    darek->on_join();
    game.add_player(std::move(isa));
    game.add_player(std::move(darek));

    MainCamera main_camera {};
    DisableCursor();
    while (!WindowShouldClose()) {
        main_camera.update(game.get_player(game.get_current_user()), GetMouseDelta());
        BeginDrawing();

        BeginMode3D(main_camera.get_camera());
        ClearBackground(BLACK);
        draw_players(game.get_current_user(), game.get_players(), main_camera);
        draw_objects(game.get_objects());
        EndMode3D();

        EndDrawing();
    }
}

void Application::draw_objects(const std::vector<std::unique_ptr<Object3d>>& objects) {
    for (const auto &object : objects)
        object->draw();
}

void Application::draw_players(std::string current_user, const std::vector<std::shared_ptr<Player>>& players, const MainCamera& main_camera) {
    for (const auto &player : players)
        player->draw(current_user, main_camera.get_mode());
}

void Application::exit() {
    CloseWindow();
}