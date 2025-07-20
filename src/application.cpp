#include <iostream>
#include <memory>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib.h"

#include "application.hpp"
#include "cube.hpp"
#include "maincamera.hpp"
#include <assert.h>

constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1280;
constexpr int FONT_SIZE = 40;

Application::Application() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PocketLife");
}

void Application::tick(Game& game, const std::vector<bool>& keys_down, MainCamera& main_camera, float tps, bool moved) {
    const auto player = game.get_player(game.get_current_user());
    if (moved) {
        PlayerMoveEvent move_event (player->get_username(), player->get_position().x, player->get_position().y, player->get_position().z);
        game.send_packet(move_event.make_packet(), false);
    }
}

void Application::run(Game& game) {
    MainCamera main_camera {};
    char ip[16] = {0};
    char port[5] = {0};
    bool ip_focus = false;
    bool port_focus = false;
    float tps = 20.0f;
    float dt_tick = 0;
    while (!WindowShouldClose()) {
        game.poll_events();
        if (!game.in_world()) {
            display_menu(game, ip, port, ip_focus, port_focus);
            continue;
        }
        float dt = GetFrameTime(); // seconds
        dt_tick += dt;
        std::string fps = std::to_string((int)round(1.0/dt));
        std::vector<bool> keys_down = {IsKeyDown(KEY_W), IsKeyDown(KEY_A), IsKeyDown(KEY_S), IsKeyDown(KEY_D), IsKeyDown(KEY_TAB)};
        const auto player = game.get_player(game.get_current_user());
        if (player == nullptr)
            continue;
        bool moved = player->move(dt, main_camera.get_direction(), main_camera.get_mode(), keys_down);
        while (dt_tick >= 1/tps) {
            tick(game, keys_down, main_camera, tps, moved);
            dt_tick -= 1/tps;
            moved = false;
        }
        main_camera.update(player, GetMouseDelta());
        BeginDrawing();

        BeginMode3D(main_camera.get_camera());
        ClearBackground(BLACK);
        draw_players(game.get_current_user(), game.get_players(), main_camera);
        draw_objects(game.get_objects());
        EndMode3D();

        int fps_size = MeasureText(fps.c_str(),FONT_SIZE);
        GuiLabel((Rectangle){0,0,fps_size,FONT_SIZE},fps.c_str());
        if (keys_down[4]) {display_scoreboard(game.get_players());}
        EndDrawing();
    }
}

void Application::display_menu(Game& game, char* ip, char* port, bool& ip_focus, bool& port_focus) {
    BeginDrawing();
    ClearBackground(BLACK);
    int text_width = MeasureText("1234567891233456", FONT_SIZE);
    Rectangle ip_box = (Rectangle) {SCREEN_WIDTH/2-text_width/2, SCREEN_HEIGHT/2-FONT_SIZE/2-(FONT_SIZE*2), text_width*1, FONT_SIZE};
    Rectangle port_box = (Rectangle){SCREEN_WIDTH/2-text_width/2, SCREEN_HEIGHT/2-FONT_SIZE/2, text_width*1, FONT_SIZE};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), ip_box)) {
        ip_focus = true;
        port_focus = false;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), port_box)) {
        ip_focus = false;
        port_focus = true;
    }

    GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);
    GuiTextBox(ip_box, ip, 15+1, ip_focus);
    GuiTextBox(port_box, port, 4+1, port_focus);
    if (GuiButton((Rectangle){SCREEN_WIDTH/2-text_width/2, SCREEN_HEIGHT/2-FONT_SIZE/2+(FONT_SIZE*2), text_width*0.4, FONT_SIZE}, "Host")) {
        if (ip[0] == '\0' || port[0] == '\0') {
        } else if (game.host("darek","test world",ip,port)) {
            DisableCursor();
        }
    } else if (GuiButton((Rectangle){SCREEN_WIDTH/2+text_width*0.1, SCREEN_HEIGHT/2-FONT_SIZE/2+(FONT_SIZE*2), text_width*0.4, FONT_SIZE}, "Join")) {
        if (ip[0] == '\0' || port[0] == '\0') {
        } else if (game.join("isabella",ip,port)) {
            DisableCursor();
        }
    }
    EndDrawing();
}

void Application::display_scoreboard(const std::vector<std::shared_ptr<Player>>& players) {
    int lineHeight = FONT_SIZE + 4;
    int y_pos = 0;
    for (int i = 0; i < players.size(); ++i) {
        if (players[i]->is_online()) {
            std::string line = players[i]->get_username() + " (" +
                        std::to_string((int)players[i]->get_position().x) + "," +
                        std::to_string((int)players[i]->get_position().y) + "," +
                        std::to_string((int)players[i]->get_position().z) + ")";
            DrawText(line.c_str(), SCREEN_WIDTH/2-MeasureText(line.c_str(), FONT_SIZE)/2, y_pos, FONT_SIZE, LIGHTGRAY);
            y_pos += lineHeight;
        }
    }
}

void Application::draw_objects(const std::vector<std::shared_ptr<Object3d>>& objects) {
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