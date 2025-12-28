#pragma once
#include <string>

class Game;
#include "raymath.h"

class Weather {
public:
    Weather(float latitude, float longitude);
    ~Weather();

    bool update();
    void set_location(float latitude, float longitude);
    void set_weather_id(int id);
    void update_sun(uint64_t current_timestamp);

    void draw(Game& game,uint64_t timestamp,uint64_t nano_seconds) const;
    void draw_post(Game& game,uint64_t timestamp,uint64_t nano_seconds,const RenderTexture2D& target) const;
    void update_weather_transform(Game& game);

    int get_weather_id() const;
    float get_latitude() const;
    float get_longitude() const;
    double get_azimuth() const;
    double get_altitude() const;
    bool is_raining() const;
    bool is_snowing() const;
    bool is_foggy() const;
private:
    Mesh rain_mesh_;
    Material rain_mat_;
    std::vector<Matrix> rain_transforms_;
    float rain_direction_[3];
    float rain_speed_;
    float rain_distance_;

    Mesh snow_mesh_;
    Material snow_mat_;
    float snow_direction_[3];
    float snow_speed_;
    float snow_distance_;
    float snow_sway_;

    Mesh quad_mesh_;
    Material quad_mat_;
    float fog_far_plane_;
    float fog_color_[4];
    
    int weather_id_;

    float latitude_;
    float longitude_;
    double azimuth_; // radians
    double altitude_; // radians
};