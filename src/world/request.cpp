#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include "world/request.hpp"
#include "logging.hpp"

std::optional<json> get_url(std::string url) {
    httplib::SSLClient cli("api.openweathermap.org", 443);
    cli.set_follow_location(true);
    auto res = cli.Get(url.c_str());
    if (!res || res->status != 200)
        return std::nullopt;
    DEBUG("Received query: " + res->body);
    try {
        auto j = nlohmann::json::parse(res->body);
        return j;
    } catch(const std::exception& e) {
        return std::nullopt;
    }
}
