#pragma once
#include <string>

class Weather {
public:
    Weather(float latitude, float longitude);
    ~Weather();

    bool update();
    void set_location(float latitude, float longitude);
    void set_weather_id(int id);
    void update_sun(uint64_t current_timestamp);

    int get_weather_id() const;
    float get_latitude() const;
    float get_longitude() const;
    double get_azimuth() const;
    double get_altitude() const;
private:
    int weather_id_;

    float latitude_;
    float longitude_;
    double azimuth_; // radians
    double altitude_; // radians
};