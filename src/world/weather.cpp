#include <cmath>
#include <fstream>

#include "json.hpp"
using json = nlohmann::json;

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "application.hpp"
#include "game.hpp"
#include "logging.hpp"
#include "world/weather.hpp"
#include "world/request.hpp"

Weather::Weather(float latitude, float longitude) : rain_distance_{20.0f},
                                                    rain_transforms_{},
                                                    rain_speed_{10.0f},
                                                    rain_direction_{0.0f,-1.0f,-0.25f},
                                                    snow_distance_{15.0f},
                                                    snow_speed_{0.6f},
                                                    snow_direction_{-0.5f,-1.0f,0.0f},
                                                    snow_sway_{0.15f},
                                                    fog_far_plane_{10000.0f},
                                                    fog_color_{0.5f,0.8f,0.9f,1.0f},
                                                    latitude_(latitude), 
                                                    longitude_(longitude) {
    weather_id_ = 800;
    azimuth_ = 0.0;
    altitude_ = 0.0;
    rain_mesh_ = GenMeshCube(0.01f,0.1f,0.01f);
    rain_mat_ = LoadMaterialDefault();
    rain_mat_.shader = Application::get_shader_rain();
    rain_mat_.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;

    snow_mesh_ = GenMeshCube(0.01f,0.01f,0.01f);
    snow_mat_ = LoadMaterialDefault();
    snow_mat_.shader = Application::get_shader_snow();
    snow_mat_.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

    quad_mesh_ = Mesh{0};
    quad_mesh_.vertexCount = 6;
    quad_mesh_.triangleCount = 2;
    quad_mesh_.vertices = (float*)MemAlloc(sizeof(float)*6*3);
    memcpy(quad_mesh_.vertices,(float[]){
        // pos    
        -1, -1, 0,
        1, -1, 0,
        1,  1, 0,

        -1, -1, 0,
        1,  1, 0,
        -1,  1, 0,
    },sizeof(float)*6*3);

    UploadMesh(&quad_mesh_,false);
    quad_mat_ = LoadMaterialDefault();
    quad_mat_.shader = Application::get_shader_fog();
    INFO("Default initialized weather");
}

Weather::~Weather() {
}

bool Weather::update() {
    DEBUG("Updating weather...");
    std::string url = "https://api.openweathermap.org/data/2.5/weather?lat=" + std::to_string(latitude_) + "&lon=" + std::to_string(longitude_) + "&appid=";
    std::ifstream file ("api.env");
    if (file) {
        std::string key {};
        std::getline(file, key);
        url += key;
    } else {
        return false;
    }
    std::optional<json> j = get_url(url);
    if (j == std::nullopt) {
        return false;
    }
    weather_id_ = j.value().at("weather")[0]["id"];
    INFO("Set weather id to: " + std::to_string(weather_id_));
    return true;
}

void Weather::update_sun(uint64_t current_timestamp) {
    constexpr auto clamp = [](double x) { return std::max(-1.0, std::min(1.0, x)); };
    // constexpr double PI = 3.14159265358979323846f;
    double julian_day = current_timestamp / 86400.0 + 2440587.5;
    double T = (julian_day - 2451545.0) / 36525.0; // julian century
    double L0 = std::fmod(280.46646 + T * (36000.76983 + T * 0.0003032), 360.0); // mean longitude
    double M = 357.52911 + T * (35999.05029 - 0.0001537 * T);
    double C = std::sin(M * PI/180.0) * (1.914602 - T * 0.004817 - 0.000014 * T)
         + std::sin(2*M * PI/180.0) * (0.019993 - 0.000101 * T)
         + std::sin(3*M * PI/180.0) * 0.000289;
    double true_long = L0 + C;
    double omega = 125.04 - 1934.136 * T;
    double apparent_long = true_long - 0.00569 - 0.00478 * std::sin(omega * PI/180.0);
    double epsilon0 = 23.43929111 - T * (0.013004167 + T * (1.6667e-7 - T * 5.0278e-7));
    double epsilon = epsilon0 + 0.00256 * std::cos(omega * PI/180.0);
    double lambda = apparent_long*PI/180.0;
    double ep = epsilon*PI/180.0;
    double alpha = (std::atan2(std::cos(ep) * std::sin(lambda), std::cos(lambda)))*180.0/PI;
    if (alpha < 0) alpha += 360.0;
    double delta = (std::asin(clamp(std::sin(ep) * std::sin(lambda))))*180.0/PI;
    double theta = 280.46061837 + 360.98564736629 * (julian_day - 2451545.0)
                 + T*T*(0.000387933 - T / 38710000.0);
    double lst = std::fmod(theta + longitude_, 360.0);
    if (lst < 0) lst += 360.0;
    double H = std::fmod(lst-alpha + 360.0, 360.0);
    double h_rad = std::asin(clamp(std::sin(latitude_*PI/180.0)*std::sin(delta*PI/180.0) +
                        std::cos(latitude_*PI/180.0)*std::cos(delta*PI/180.0)*std::cos(H*PI/180.0)));
    double A_rad = std::acos(clamp((std::sin(delta*PI/180.0) - std::sin(h_rad)*std::sin(latitude_*PI/180.0)) /
                        (std::cos(h_rad)*std::cos(latitude_*PI/180.0))));
    azimuth_ = std::sin(H*PI/180.0) > 0 ? 2*PI - A_rad : A_rad;
    altitude_ = h_rad;
    INFO("Azimuth: " + std::to_string(azimuth_*180.0/PI) + " Altitude: " + std::to_string(altitude_*180.0/PI));
}

void Weather::draw(Game& game, uint64_t timestamp, uint64_t nano_seconds) const {
    float ss = nano_seconds/(1e9f);
    uint32_t seconds = timestamp%(10000);
    Vector3 pos = game.get_current_player()->get().get_position();
    float pos_array[3] = {pos.x,pos.y,pos.z};
    
    if (is_raining()) {
        int s_loc = GetShaderLocation(rain_mat_.shader,"seconds");
        SetShaderValue(rain_mat_.shader,s_loc,&seconds,SHADER_UNIFORM_INT);
        int ss_loc = GetShaderLocation(rain_mat_.shader,"subseconds");
        SetShaderValue(rain_mat_.shader,ss_loc,&ss,SHADER_UNIFORM_FLOAT);
        int speed_loc = GetShaderLocation(rain_mat_.shader,"rainSpeed");
        SetShaderValue(rain_mat_.shader,speed_loc,&rain_speed_,SHADER_UNIFORM_FLOAT);
        int rd_loc = GetShaderLocation(rain_mat_.shader,"rainDirection");
        SetShaderValue(rain_mat_.shader,rd_loc,rain_direction_,SHADER_UNIFORM_VEC3);
        int player_loc = GetShaderLocation(rain_mat_.shader,"playerPosition");
        SetShaderValue(rain_mat_.shader,player_loc,pos_array,SHADER_UNIFORM_VEC3);

        // for (int i = 0; i < rain_transforms_.size(); i++) {
        //     DrawMesh(rain_mesh_,rain_mat_,rain_transforms_[i]);
        // }
        DrawMeshInstanced(rain_mesh_,rain_mat_,rain_transforms_.data(),rain_transforms_.size());
    } else if (is_snowing()) {
        int s_loc = GetShaderLocation(snow_mat_.shader,"seconds");
        SetShaderValue(snow_mat_.shader,s_loc,&seconds,SHADER_UNIFORM_INT);
        int ss_loc = GetShaderLocation(snow_mat_.shader,"subseconds");
        SetShaderValue(snow_mat_.shader,ss_loc,&ss,SHADER_UNIFORM_FLOAT);
        int speed_loc = GetShaderLocation(snow_mat_.shader,"snowSpeed");
        SetShaderValue(snow_mat_.shader,speed_loc,&snow_speed_,SHADER_UNIFORM_FLOAT);
        int sway_loc = GetShaderLocation(snow_mat_.shader,"snowSway");
        SetShaderValue(snow_mat_.shader,sway_loc,&snow_sway_,SHADER_UNIFORM_FLOAT);
        int rd_loc = GetShaderLocation(snow_mat_.shader,"snowDirection");
        SetShaderValue(snow_mat_.shader,rd_loc,snow_direction_,SHADER_UNIFORM_VEC3);
        int player_loc = GetShaderLocation(snow_mat_.shader,"playerPosition");
        SetShaderValue(snow_mat_.shader,player_loc,pos_array,SHADER_UNIFORM_VEC3);

        // for (int i = 0; i < rain_transforms_.size(); i++) {
        //     DrawMesh(rain_mesh_,rain_mat_,rain_transforms_[i]);
        // }
        DrawMeshInstanced(snow_mesh_,snow_mat_,rain_transforms_.data(),rain_transforms_.size());
    }
} 

void Weather::draw_post(Game& game,uint64_t timestamp,uint64_t nano_seconds,const RenderTexture2D& target) const {
    SetShaderValueTexture(quad_mat_.shader, GetShaderLocation(quad_mat_.shader, "depthTexture"), target.depth);
    SetShaderValueTexture(quad_mat_.shader, GetShaderLocation(quad_mat_.shader, "colorTexture"), target.texture);
    SetShaderValue(quad_mat_.shader,GetShaderLocation(quad_mat_.shader,"colorTexture"),&target.texture,SHADER_UNIFORM_SAMPLER2D);
    SetShaderValue(quad_mat_.shader,GetShaderLocation(quad_mat_.shader,"depthTexture"),&target.depth,SHADER_UNIFORM_SAMPLER2D);
    SetShaderValue(quad_mat_.shader,GetShaderLocation(quad_mat_.shader,"fogFarPlane"),&fog_far_plane_,SHADER_UNIFORM_FLOAT);
    SetShaderValue(quad_mat_.shader,GetShaderLocation(quad_mat_.shader,"fogColor"),fog_color_,SHADER_UNIFORM_VEC4);
    DrawMesh(quad_mesh_,quad_mat_,MatrixIdentity());
}

void Weather::update_weather_transform(Game& game) {
    // https://openweathermap.org/weather-conditions
    INFO("Updating rain matrices for instancing...");
    constexpr int RADIUS = 30;
    constexpr int SIDE_LENGTH = RADIUS*2*5;
    Vector3 center = game.get_current_player()->get().get_position();
    int transform_count = is_raining()||is_snowing() ? SIDE_LENGTH*SIDE_LENGTH : 0;
    rain_transforms_.resize(transform_count);
    INFO("Resizing to " + std::to_string(transform_count) + " matrices");

    Vector3 direction_vec;
    float* direction;
    float distance = is_snowing() ? snow_distance_ : rain_distance_;
    if (is_snowing()) {
        fog_far_plane_ = 150.0f;
        fog_color_[0] = 0.9f;
        fog_color_[1] = 0.9f;
        fog_color_[2] = 0.9f;
        fog_color_[3] = 1.0f;
        direction = snow_direction_;
        direction_vec = Vector3{snow_direction_[0],snow_direction_[1],snow_direction_[2]};
    } else if (is_raining()) {
        fog_far_plane_ = 75.0f;
        fog_color_[0] = 0.1f;
        fog_color_[1] = 0.15f;
        fog_color_[2] = 0.4f;
        fog_color_[3] = 1.0f;
        direction = rain_direction_;
        direction_vec = Vector3{rain_direction_[0],rain_direction_[1],rain_direction_[2]};
    } else if (is_foggy()) {
        fog_far_plane_ = 50.0f;
        fog_color_[0] = 0.7f;
        fog_color_[1] = 0.7f;
        fog_color_[2] = 0.7f;
        fog_color_[3] = 1.0f;
        return;
    } else if (!transform_count) {
        fog_far_plane_ = 10000.0f;
        fog_color_[0] = 0.5f;
        fog_color_[1] = 0.8f;
        fog_color_[2] = 0.9f;
        fog_color_[3] = 1.0f;
        return;
    }
    Quaternion q = QuaternionFromVector3ToVector3(Vector3{0.0f,-1.0f,0.0f},Vector3Normalize(direction_vec));
    Matrix rotate = QuaternionToMatrix(q);
    for (int i = 0; i < SIDE_LENGTH; i++) {
        for (int j = 0; j < SIDE_LENGTH; j++) {
            int indice = i*SIDE_LENGTH + j;
            Vector3 pos = Vector3{(i-SIDE_LENGTH/2)*0.2f-direction_vec.x*distance,0.0f,(j-SIDE_LENGTH/2)*0.2f-direction_vec.z*distance} + Vector3Scale(Vector3Normalize(Vector3Negate(direction_vec)),distance);
            rain_transforms_[indice] = MatrixMultiply(rotate,MatrixTranslate(pos.x,pos.y,pos.z));
        }
    }
}

void Weather::set_location(float latitude, float longitude) {
    latitude_ = latitude;
    longitude_ = longitude;
}

void Weather::set_weather_id(int id) {
    weather_id_ = id;
}

int Weather::get_weather_id() const {
    return weather_id_;
}

float Weather::get_latitude() const {return latitude_;}
float Weather::get_longitude() const {return longitude_;}
double Weather::get_azimuth() const {return azimuth_;}
double Weather::get_altitude() const {return altitude_;}
bool Weather::is_raining() const {return weather_id_/100 == 2 || weather_id_/100 == 5 || weather_id_/100 == 3;}
bool Weather::is_snowing() const {return weather_id_/100 == 6;}
bool Weather::is_foggy() const {return weather_id_/100 == 7;}