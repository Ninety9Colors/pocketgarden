#include <iostream>
#include <fstream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "json.hpp"

#include "weather.hpp"

Weather::Weather(float latitude, float longitude) : latitude_(latitude), longitude_(longitude) {
    weather_id_ = 800;
    sunrise_ = 0;
    sunset_ = 0;
}

Weather::~Weather() {
}

bool Weather::update() {
    std::string url = "https://api.openweathermap.org/data/2.5/weather?lat=" + std::to_string(latitude_) + "&lon=" + std::to_string(longitude_) + "&appid=";
    std::ifstream file ("api.env");
    if (file) {
        std::string key {};
        std::getline(file, key);
        url += key;
    } else {
        return false;
    }
    httplib::SSLClient cli("api.openweathermap.org", 443);
    cli.set_follow_location(true);
    auto res = cli.Get(url.c_str());
    if (!res || res->status != 200)
        return false;
    std::cout << res->body << "\n";
    try {
        auto json = nlohmann::json::parse(res->body);
        weather_id_ = json["weather"][0]["id"];
        sunrise_ = json["sys"]["sunrise"];
        sunset_ = json["sys"]["sunset"];
        std::cout << "Set weather id to: " << weather_id_ << "\n";
    } catch(const std::exception& e) {
        return false;
    }
    return true;
}

void Weather::set_location(float latitude, float longitude) {
    latitude_ = latitude;
    longitude_ = longitude;
}

void Weather::set_suntimes(int64_t sunrise, int64_t sunset) {
    sunrise_ = sunrise;
    sunset_ = sunset;
}

void Weather::set_weather_id(int id) {
    weather_id_ = id;
}

int Weather::get_weather_id() const {
    return weather_id_;
}

float Weather::get_latitude() const {return latitude_;}
float Weather::get_longitude() const {return longitude_;}
int64_t Weather::get_sunrise() const {return sunrise_;}
int64_t Weather::get_sunset() const {return sunset_;}