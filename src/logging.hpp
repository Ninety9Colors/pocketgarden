#pragma once

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef POCKETLIFE_DEBUG
    #define INFO(x) (log(1,(x)))
    #define DEBUG(x) (log(2,(x)))
    #define WARN(x) (log(3,(x)))
    #define CRITICAL(x) (log(4,(x)))
#else
    #define INFO(x) ((void)0)
    #define DEBUG(x) ((void)0)
    #define WARN(x) ((void)0)
    #define CRITICAL(x) ((void)0)
#endif

const std::string LOG_PATH = "logs/";

inline void log(const int level, const std::string message) {
    const std::chrono::time_point now(std::chrono::system_clock::now());
    const auto local = std::chrono::current_zone()->to_local(now);
    const std::chrono::year_month_day ymd(std::chrono::floor<std::chrono::days>(local));

    std::ostringstream oss;
    oss << ymd;
    const std::string date = oss.str();
    oss.str("");
    oss << std::format("{:%T}",std::chrono::floor<std::chrono::seconds>(local));
    const std::string time = oss.str();

    std::ofstream fout;
    fout.open(LOG_PATH + date + ".log",std::ios::app);

    std::cout << time << " ";
    fout << time << " ";

    switch (level) {
    case 1:
        fout << "INFO ";
        std::cout << "\033[37mINFO\033[0m ";
        break;
    case 2:
        fout << "DEBUG ";
        std::cout << "\033[1;37mDEBUG\033[0m ";
        break;
    case 3:
        fout << "WARN ";
        std::cout << "\033[1;33mWARN\033[0m ";
        break;
    case 4:
        fout << "CRITICAL ";
        std::cout << "\033[1;31mCRITICAL\033[0m ";
        break;
    }

    std::cout << message << std::endl;
    fout << message << std::endl;

    fout.close();
}