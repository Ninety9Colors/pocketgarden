#pragma once
#include <string>

class Weather {
public:
    Weather(float latitude, float longitude);
    ~Weather();

    bool update();
    void set_location(float latitude, float longitude);
    void set_suntimes(int64_t sunrise, int64_t sunset);
    void set_weather_id(int id);

    int get_weather_id() const;
    float get_latitude() const;
    float get_longitude() const;
    int64_t get_sunrise() const;
    int64_t get_sunset() const;
private:
    int weather_id_;

    int64_t sunrise_;
    int64_t sunset_;
    float latitude_;
    float longitude_;
};