#pragma once
#include <string>

class Game;

class Weather {
public:
    Weather(float latitude, float longitude);
    ~Weather();

    bool update();
    void set_location(float latitude, float longitude);
    void set_weather_id(int id);
    void update_sun(uint64_t current_timestamp);

    void draw(Game& game,uint64_t timestamp,uint64_t nano_seconds) const;

    int get_weather_id() const;
    float get_latitude() const;
    float get_longitude() const;
    double get_azimuth() const;
    double get_altitude() const;
private:
    Mesh rain_mesh_;
    Material rain_mat_;
    
    int weather_id_;

    float latitude_;
    float longitude_;
    double azimuth_; // radians
    double altitude_; // radians
};