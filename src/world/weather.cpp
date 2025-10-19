#include <cmath>
#include <iostream>
#include <fstream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "json.hpp"

#include "world/weather.hpp"

Weather::Weather(float latitude, float longitude) : latitude_(latitude), longitude_(longitude) {
    weather_id_ = 800;
    azimuth_ = 0.0;
    altitude_ = 0.0;
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
        std::cout << "Set weather id to: " << weather_id_ << "\n";
    } catch(const std::exception& e) {
        return false;
    }
    return true;
}

void Weather::update_sun(uint64_t current_timestamp) {
    constexpr auto clamp = [](double x) { return std::max(-1.0, std::min(1.0, x)); };
    constexpr double PI = 3.14159265358979323846f;
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
    std::cout << "Azimuth: " << std::to_string(azimuth_*180.0/PI) << " Altitude: " << std::to_string(altitude_*180.0/PI) << "\n";
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