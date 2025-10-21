#include <assert.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib.h"

#include "application.hpp"
#include "logging.hpp"
#include "object/consistent/cube.hpp"
#include "player/maincamera.hpp"
#include "object/consistent/move_tool.hpp"
#include "object/procedural/spline.hpp"

constexpr int DEFAULT_SCREEN_WIDTH = 1280;
constexpr int DEFAULT_SCREEN_HEIGHT = 720;
constexpr int FONT_SIZE = 40;

Application::Application() : ip_({0}), port_({0}), username_({0}), ip_focus_(false), port_focus_(false), username_focus_(false) {
    DEBUG("Initializing window with size " + std::to_string(DEFAULT_SCREEN_WIDTH) + "," + std::to_string(DEFAULT_SCREEN_HEIGHT));
    InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "PocketGarden");
    SetWindowSize(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
    SetExitKey(KEY_NULL);
    event_buffer_ = {};
    shader_default_ = std::shared_ptr<Shader>(
        new Shader(LoadShader("shaders/default.vs","shaders/default.fs")),
        [](Shader* s) {
            UnloadShader(*s);
            delete s;
        }
    );
    shader_default_->locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(*shader_default_, "colorDiffuse");
    shader_light_source_ = std::shared_ptr<Shader>(
        new Shader(LoadShader("shaders/default.vs","shaders/light_source.fs")),
        [](Shader* s) {
            UnloadShader(*s);
            delete s;
        }
    );
}

void Application::tick(std::map<std::string, std::shared_ptr<Event>>& event_buffer, Game& game) {
    const auto player = game.get_current_player();
    for (const auto& p : event_buffer) {
        game.get_network()->send_packet(p.second->make_packet(), p.second->reliable());
    }
    event_buffer.clear();
}

void Application::run(Game& game) {
    DEBUG("Starting application...");
    MainCamera main_camera {};

    const float TPS = 20.0f; // ticks per second
    float dt_tick = 0; // time elapsed since last tick (seconds)
    uint64_t total_ticks = 0;

    const int WEATHER_UPDATE_INTERVAL = 300; // (seconds)
    uint64_t last_weather_update = 0;

    const int UI_UPDATE_INTERVAL = 1; // (seconds)
    uint64_t last_ui_update = 0;

    int sun_position_loc = GetShaderLocation(*shader_default_,"sunPos");
    int sun_color_loc = GetShaderLocation(*shader_default_,"sunColor");
    int ambient_loc = GetShaderLocation(*shader_default_,"ambient");

    std::string fps_buffer;

    while (!WindowShouldClose()) {
        if (!game.in_world()) {
            display_menu(game);
            continue;
        }

        uint64_t current_timestamp = std::time(nullptr);
        float dt = GetFrameTime(); // elapsed seconds of last frame (seconds)
        dt_tick += dt;

        std::vector<bool> keybinds = {IsKeyDown(KEY_W), IsKeyDown(KEY_A), IsKeyDown(KEY_S), IsKeyDown(KEY_D), IsKeyDown(KEY_TAB), IsKeyDown(KEY_ESCAPE),
                                        IsMouseButtonPressed(MOUSE_LEFT_BUTTON), (GetMouseWheelMoveV().y > 0), (GetMouseWheelMoveV().y < 0), IsKeyPressed(KEY_SPACE), IsKeyDown(KEY_Q), IsKeyDown(KEY_E), IsKeyPressed(KEY_R)};
        game.poll_events(game.get_current_user(), game.get_world(), game.get_network(), game, current_timestamp, event_buffer_, main_camera, keybinds,dt,shader_default_);
        
        const auto player = game.get_current_player();
        if (player == nullptr) {
            CRITICAL("Could not find player!");
            break;
        }

        player->update(event_buffer_, main_camera, game.get_world(), keybinds, dt);
        if (dt_tick >= 1.0f/TPS) {
            if (current_timestamp-last_weather_update >= WEATHER_UPDATE_INTERVAL && game.get_network()->is_host()) {
                bool updated = game.get_world()->get_weather()->update();
                if (updated) {
                    DEBUG("Updating weather information in world and shader...");
                    event_buffer_["WeatherUpdateEvent"] = std::make_shared<WeatherUpdateEvent>(game.get_world()->get_weather()->get_weather_id());
                    last_weather_update = current_timestamp;
                    game.get_world()->get_weather()->update_sun(current_timestamp);
                    game.get_world()->update_sun();

                    // Pass new lighting information to shader
                    const Vector3 sun_position = game.get_world()->get_sun()->get_position();
                    float sun_pos[3] = {sun_position.x, sun_position.y, sun_position.z};
                    SetShaderValue(*shader_default_, sun_position_loc, sun_pos, SHADER_UNIFORM_VEC3);
                    float sun_color[4] = {1.0f,1.0f,(std::pow(std::max(sun_position.y,0.0f),2)/10000.0f),1.0f};
                    SetShaderValue(*shader_default_, sun_color_loc, sun_color, SHADER_UNIFORM_VEC4);

                    float ambient_level = (std::pow(std::max(sun_position.y,0.0f),2)/10000.0f)*0.5f + 0.25f;
                    float ambient[4] = {ambient_level,ambient_level,ambient_level,1.0f};
                    SetShaderValue(*shader_default_, ambient_loc, ambient, SHADER_UNIFORM_VEC4);
                }
            }
            tick(event_buffer_, game);
            dt_tick = 0;
            total_ticks++;
        }
        main_camera.update(player, GetMouseDelta());

        float cam_pos[3] = {main_camera.get_position().x, main_camera.get_position().y, main_camera.get_position().z};
        SetShaderValue(*shader_default_, shader_default_->locs[SHADER_LOC_VECTOR_VIEW], cam_pos, SHADER_UNIFORM_VEC3);

        BeginDrawing();

        BeginMode3D(main_camera.get_camera());
        ClearBackground(SKYBLUE);
        game.get_world()->get_sun()->draw();
        draw_players(game.get_current_user(), game.get_world()->get_players(), main_camera);
        draw_objects(game.get_world()->get_objects());
        EndMode3D();

        // Crosshair
        DrawCircle(GetScreenWidth()/2,GetScreenHeight()/2,3,WHITE);

        // Debug UI
        if (current_timestamp-last_ui_update >= UI_UPDATE_INTERVAL) {
            INFO("Updating FPS");
            fps_buffer = std::to_string((int)round(1.0f/dt));
            last_ui_update = current_timestamp;
        }
        int fps_size = MeasureText(fps_buffer.c_str(),FONT_SIZE);
        GuiLabel((Rectangle){0,0,fps_size,FONT_SIZE},fps_buffer.c_str());

        if (keybinds[4]) {display_scoreboard(game.get_world()->get_players());}
        EndDrawing();
        if (keybinds[5]) {game.disconnect();}
    }
    game.disconnect();
}

void Application::display_menu(Game& game) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    BeginDrawing();
    ClearBackground(BLACK);
    int text_width = MeasureText("1234567891233456", FONT_SIZE);
    Rectangle ip_box = (Rectangle) {width/2-text_width/2, height/2-FONT_SIZE/2-(FONT_SIZE*2), text_width*1, FONT_SIZE};
    Rectangle port_box = (Rectangle){width/2-text_width/2, height/2-FONT_SIZE/2, text_width*1, FONT_SIZE};
    Rectangle username_box = (Rectangle){width/2-text_width/2, height/2-FONT_SIZE/2-(FONT_SIZE*2)*2, text_width*1, FONT_SIZE};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), ip_box)) {
        ip_focus_ = true;
        port_focus_ = false;
        username_focus_ = false;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), port_box)) {
        ip_focus_ = false;
        port_focus_ = true;
        username_focus_ = false;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), username_box)) {
        ip_focus_ = false;
        port_focus_ = false;
        username_focus_ = true;
    }

    GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);
    GuiTextBox(ip_box, ip_, 15+1, ip_focus_);
    GuiTextBox(port_box, port_, 5+1, port_focus_);
    GuiTextBox(username_box, username_, 15+1, username_focus_);
    if (GuiButton((Rectangle){width/2-text_width/2, height/2-FONT_SIZE/2+(FONT_SIZE*2), text_width*0.4, FONT_SIZE}, "Host")) {
        if (ip_[0] == '\0' || port_[0] == '\0' || username_[0] == '\0') {
        } else if (game.host((const char*)username_,"test world.data",ip_,port_,shader_default_)) {
            DisableCursor();
            event_buffer_.clear();
        }
    } else if (GuiButton((Rectangle){width/2+text_width*0.1, height/2-FONT_SIZE/2+(FONT_SIZE*2), text_width*0.4, FONT_SIZE}, "Join")) {
        if (ip_[0] == '\0' || port_[0] == '\0' || username_[0] == '\0') {
        } else if (game.join((const char*)username_,ip_,port_)) {
            DisableCursor();
            event_buffer_.clear();
        }
    }
    EndDrawing();
}

void Application::display_scoreboard(const std::vector<std::shared_ptr<Player>>& players) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int lineHeight = FONT_SIZE + 4;
    int y_pos = 0;
    for (int i = 0; i < players.size(); ++i) {
        if (players[i]->is_online()) {
            std::string line = players[i]->get_username() + " (" +
                        std::to_string((int)players[i]->get_position().x) + "," +
                        std::to_string((int)players[i]->get_position().y) + "," +
                        std::to_string((int)players[i]->get_position().z) + ")";
            DrawText(line.c_str(), width/2-MeasureText(line.c_str(), FONT_SIZE)/2, y_pos, FONT_SIZE, LIGHTGRAY);
            y_pos += lineHeight;
        }
    }
}

void Application::draw_objects(const std::map<uint32_t, std::shared_ptr<Object3d>>& objects) {
    for (const auto& p : objects) {
        p.second->draw();
    }
}

void Application::draw_players(std::string current_user, const std::vector<std::shared_ptr<Player>>& players, const MainCamera& main_camera) {
    for (const auto &player : players) {
        player->draw(current_user, main_camera);
    }
}

void Application::exit() {
    DEBUG("Closing Window");
    CloseWindow();
}

std::map<std::string, std::shared_ptr<Event>>& Application::get_event_buffer() {
    return event_buffer_;
}