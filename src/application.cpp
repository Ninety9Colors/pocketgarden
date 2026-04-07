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
#include "rlgl.h"
#include "json.hpp"
using json = nlohmann::json;

#include "application.hpp"
#include "logging.hpp"
#include "object/consistent/cube.hpp"
#include "player/maincamera.hpp"
#include "object/consistent/move_tool.hpp"
#include "object/procedural/spline.hpp"
#include "settings.hpp"

#include "util/parse.hpp"
#include "util/draw.hpp"
#include "object/procedural/marching_cubes.hpp"
#include "object/procedural/voxel_thinning.hpp"

static RenderTexture2D LoadRenderTextureDepthTex(int width, int height);
static void UnloadRenderTextureDepthTex(RenderTexture2D target);

constexpr int DEFAULT_SCREEN_WIDTH = 1280;
constexpr int DEFAULT_SCREEN_HEIGHT = 720;
constexpr int FONT_SIZE = 40;

Shader Application::shader_default_ {0};
Shader Application::shader_instanced_ {0};
Shader Application::shader_rain_ {0};
Shader Application::shader_snow_ {0};
Shader Application::shader_fog_ {0};
Application::Application() : ip_({0}), port_({0}), username_({0}), ip_focus_(false), port_focus_(false), username_focus_(false) {
    // TODO: Load settings from file
    Settings::set("Camera Sensitivity",0.001f);
    Settings::set("Log Level", 1);
    
    DEBUG("Initializing window with size " + std::to_string(DEFAULT_SCREEN_WIDTH) + "," + std::to_string(DEFAULT_SCREEN_HEIGHT));
    InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "PocketGarden");
    SetWindowSize(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
    SetExitKey(KEY_NULL);
    shader_default_ = LoadShader("shaders/default.vs","shaders/default.fs");
    shader_default_.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader_default_, "colorDiffuse");

    shader_instanced_ = LoadShader("shaders/instanced.vs","shaders/default.fs");
    shader_instanced_.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader_instanced_,"viewPos");
    shader_instanced_.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader_instanced_, "instanceTransform");
    shader_instanced_.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader_instanced_, "colorDiffuse");

    shader_fog_ = LoadShader("shaders/fog.vs","shaders/fog.fs");

    shader_rain_ = LoadShader("shaders/rain.vs","shaders/rain.fs");
    shader_rain_.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader_rain_, "colorDiffuse");
    shader_rain_.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader_rain_, "instanceTransform");

    shader_snow_ = LoadShader("shaders/snow.vs","shaders/snow.fs");
    shader_snow_.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader_snow_, "colorDiffuse");
    shader_snow_.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader_snow_, "instanceTransform");
}

void Application::run(Game& game) {
    auto coords = parse_xyz("dryopteris-erythrosora-01 (1).xyz",true);
    double voxel_size;
    std::cout << "Enter voxel size as a double: ";
    std::cin >> voxel_size;
    double display_voxel_size;
    std::cout << "Enter display_voxel_size size as a double: ";
    std::cin >> display_voxel_size;
    auto voxels = voxelize_pcd(coords.first,coords.second,voxel_size);
    auto voxel_grid = voxels.first.first;
    std::vector<Mesh> pcd_mesh = march_cubes(voxels.first.first,voxels.first.second,voxels.second,display_voxel_size,0.0);
    for (int i = 0; i < pcd_mesh.size(); i++)
        UploadMesh(&pcd_mesh[i],false);
    thin_voxels(voxel_grid);

    DEBUG("Starting application...");

    const float TPS = 20.0f; // ticks per second
    float dt_tick = 0; // time elapsed since last tick (seconds)
    uint64_t total_ticks = 0;

    const int WEATHER_UPDATE_INTERVAL = 300; // (seconds)
    uint64_t last_weather_update = 0;

    const int UI_UPDATE_INTERVAL = 1; // (seconds)
    uint64_t last_ui_update = 0;

    int sun_position_loc = GetShaderLocation(shader_default_,"sunPos");
    int sun_color_loc = GetShaderLocation(shader_default_,"sunColor");
    int ambient_loc = GetShaderLocation(shader_default_,"ambient");

    int sun_position_loc_instanced = GetShaderLocation(shader_instanced_,"sunPos");
    int sun_color_loc_instanced = GetShaderLocation(shader_instanced_,"sunColor");
    int ambient_loc_instanced = GetShaderLocation(shader_instanced_,"ambient");

    SetShaderValue(shader_default_, sun_position_loc, (float[3]){0.0f,game.get_world().get_sun().get_position().y,0.0f}, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_default_, sun_color_loc, (float[4]){1.0f,1.0f,1.0f,1.0f}, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader_default_, ambient_loc, (float[4]){1.0f,1.0f,0.75f,1.0f}, SHADER_UNIFORM_VEC4);

    SetShaderValue(shader_instanced_, sun_position_loc_instanced, (float[3]){0.0f,game.get_world().get_sun().get_position().y,0.0f}, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_instanced_, sun_color_loc_instanced, (float[4]){1.0f,1.0f,1.0f,1.0f}, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader_instanced_, ambient_loc_instanced, (float[4]){1.0f,1.0f,0.75f,1.0f}, SHADER_UNIFORM_VEC4);

    std::string fps_buffer;

    RenderTexture2D target = LoadRenderTextureDepthTex(DEFAULT_SCREEN_WIDTH,DEFAULT_SCREEN_HEIGHT);

    while (!WindowShouldClose()) {
        if (!game.in_world()) {
            display_menu(game);
            continue;
        }
        uint64_t current_timestamp = std::time(nullptr);
        uint64_t nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now()-std::chrono::floor<std::chrono::seconds>(std::chrono::high_resolution_clock::now())).count();
        float dt = GetFrameTime(); // elapsed seconds of last frame (seconds)
        dt_tick += dt;

        std::vector<bool> keybinds = {IsKeyDown(KEY_W), IsKeyDown(KEY_A), IsKeyDown(KEY_S), IsKeyDown(KEY_D), IsKeyDown(KEY_TAB), IsKeyDown(KEY_ESCAPE),
                                        IsMouseButtonPressed(MOUSE_LEFT_BUTTON), (GetMouseWheelMoveV().y > 0), (GetMouseWheelMoveV().y < 0), IsKeyPressed(KEY_SPACE), IsKeyDown(KEY_Q), IsKeyDown(KEY_E), IsKeyPressed(KEY_R)};
        game.poll_events();
        if (game.get_world().get_player(game.get_current_username()) == std::nullopt) {
            WARN("Could not find current username: " + game.get_current_username() + ", could be waiting on world SyncEvent");
            game.tick(keybinds,current_timestamp,dt);
            continue;
        }
        game.update_current_player(keybinds,dt);
        game.update_main_camera(GetMouseDelta());
        if (dt_tick >= 1.0f/TPS) {
            if (current_timestamp-last_weather_update >= WEATHER_UPDATE_INTERVAL && game.get_network().is_host()) {
                bool updated = game.get_world().get_weather().update();
                if (updated) {
                    DEBUG("Updating weather information in world and shader...");
                    game.queue_event_send(std::make_unique<WeatherUpdateEvent>(game.get_world().get_weather().get_weather_id()));
                    game.get_world().get_weather().update_sun(current_timestamp);
                    game.get_world().update_sun();
                    game.get_world().get_weather().update_weather_transform(game);
                } else {
                    WARN("Failed to retrieve weather information");
                }
                last_weather_update = current_timestamp;
            }
            game.tick(keybinds,current_timestamp,dt);
            dt_tick = 0;
            total_ticks++;
        }
        float cam_pos[3] = {game.get_camera().get_position().x, game.get_camera().get_position().y, game.get_camera().get_position().z};
        SetShaderValue(shader_default_, shader_default_.locs[SHADER_LOC_VECTOR_VIEW], cam_pos, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader_instanced_, shader_instanced_.locs[SHADER_LOC_VECTOR_VIEW], cam_pos, SHADER_UNIFORM_VEC3);
        
        BeginTextureMode(target);
            ClearBackground(SKYBLUE);
            // Draw Calls
            BeginMode3D(game.get_camera().get_camera());
            game.get_world().get_weather().draw(game,current_timestamp,nanoseconds);
            game.get_world().get_sun().draw(game);
            draw_players(game,game.get_current_username(), game.get_world().get_players());
            draw_objects(game,game.get_world().get_objects());
            // for (int i = 0; i < pcd_mesh.size(); i++)
            //     DrawMesh(pcd_mesh[i],LoadMaterialDefault(),MatrixIdentity());
            draw_binary_voxels(voxel_grid,0.1f);
            EndMode3D();
        EndTextureMode();

        BeginDrawing();
        // Post processing
        game.get_world().get_weather().draw_post(game,current_timestamp,nanoseconds,target);
        // Crosshair
        DrawCircle(GetScreenWidth()/2,GetScreenHeight()/2,3,WHITE);

        // Debug UI
        if (current_timestamp-last_ui_update >= UI_UPDATE_INTERVAL) {
            fps_buffer = std::to_string((int)round(1.0f/dt));
            last_ui_update = current_timestamp;
        }
        int fps_size = MeasureText(fps_buffer.c_str(),FONT_SIZE);
        GuiLabel((Rectangle){0,0,fps_size,FONT_SIZE},fps_buffer.c_str());

        if (keybinds[4]) {display_scoreboard(game.get_world().get_players());}
        EndDrawing();
        if (keybinds[5]) {game.close_game();}
    }
    game.close_game();
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
        } else if (game.host((const char*)username_,"test world.json",ip_,port_)) {
            DisableCursor();
        }
    } else if (GuiButton((Rectangle){width/2+text_width*0.1, height/2-FONT_SIZE/2+(FONT_SIZE*2), text_width*0.4, FONT_SIZE}, "Join")) {
        if (ip_[0] == '\0' || port_[0] == '\0' || username_[0] == '\0') {
        } else if (game.join((const char*)username_,ip_,port_)) {
            DisableCursor();
        }
    }
    EndDrawing();
}

void Application::display_scoreboard(const std::vector<Player>& players) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int lineHeight = FONT_SIZE + 4;
    int y_pos = 0;
    for (int i = 0; i < players.size(); ++i) {
        if (players[i].is_online()) {
            std::string line = players[i].get_username() + " (" +
                        std::to_string((int)players[i].get_position().x) + "," +
                        std::to_string((int)players[i].get_position().y) + "," +
                        std::to_string((int)players[i].get_position().z) + ")";
            DrawText(line.c_str(), width/2-MeasureText(line.c_str(), FONT_SIZE)/2, y_pos, FONT_SIZE, LIGHTGRAY);
            y_pos += lineHeight;
        }
    }
}

void Application::draw_objects(Game& game, const std::map<uint32_t, std::unique_ptr<Object3d>>& objects) const {
    for (const auto& p : objects) {
        p.second->draw(game);
    }
}
void Application::draw_objects(Game& game, const std::map<uint32_t, std::unique_ptr<Object3d>>& objects, Material material) const {
    for (const auto& p : objects) {
        p.second->draw(game, material);
    }
}
void Application::draw_players(Game& game, std::string current_user, const std::vector<Player>& players) const {
    for (const auto &player : players) {
        player.draw(game);
    }
}
void Application::draw_players(Game& game, std::string current_user, const std::vector<Player>& players, Material material) const {
    for (const auto &player : players) {
        player.draw(game, material);
    }
}

void Application::exit() {
    DEBUG("Closing Window");
    CloseWindow();
}

static RenderTexture2D LoadRenderTextureDepthTex(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(0, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        // Create depth texture buffer (instead of raylib default renderbuffer)
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       // DEPTH_COMPONENT_24BIT: Not defined in raylib
        target.depth.mipmaps = 1;

        // Attach color texture and depth texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) INFO("FBO: [ID "+ std::to_string(target.id) + "] Framebuffer object created successfully");

        rlDisableFramebuffer();
    }
    else WARN("FBO: Framebuffer object can not be created");

    return target;
}

static void UnloadRenderTextureDepthTex(RenderTexture2D target)
{
    if (target.id > 0)
    {
        // Color texture attached to FBO is deleted
        rlUnloadTexture(target.texture.id);
        rlUnloadTexture(target.depth.id);

        // NOTE: Depth texture is automatically
        // queried and deleted before deleting framebuffer
        rlUnloadFramebuffer(target.id);
    }
}