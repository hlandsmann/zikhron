#include "spaced_repetition/DataBase.h"
#include <annotation/TextCard.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <spaced_repetition/TreeWalker.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "config.json";
    return std::make_shared<zikhron::Config>(path_to_exe.parent_path());
    // std::ifstream ifs(zikhron_cfg_json);
    // if (not ifs.is_open()) {
    //     spdlog::error("Failed to open {}", zikhron_cfg_json.c_str());
    // }
    // return nlohmann::json::parse(ifs);
}

auto loadCardDB(const fs::path& card_db_path) -> std::shared_ptr<CardDB>
{
    auto cardDB = std::make_shared<CardDB>();
    try {
        cardDB->loadFromDirectory(card_db_path / "cards");
    } catch (const std::exception& e) {
        spdlog::error("Exception: {}", e.what());
        std::quick_exit(1);
    } catch (...) {
        spdlog::error("Unknown Error, load Card Database failed!");
        std::quick_exit(1);
    }
    return cardDB;
}

auto main() -> int
{
    auto zikhron_cfg = get_zikhron_cfg();
    auto database = std::make_shared<DataBase>(zikhron_cfg);
    // fs::path dictionary_fn = zikhron_cfg["dictionary"];
    // fs::path card_db_path = zikhron_cfg["database_directory"];
    //
    // auto zh_dictionary = std::make_shared<ZH_Dictionary>(dictionary_fn);
    // auto cardDB = loadCardDB(card_db_path);
    // spdlog::info("CardDB size: {}", cardDB->get().size());
    // TreeWalker treeWalker{cardDB, zh_dictionary};
    return 0;
}

//
// #include <bits/chrono.h>
// #include <spdlog/spdlog.h>
// #include <chrono>
// #include <ctime>
// #include <string>
// #include <string_view>
//
// namespace chrono = std::chrono;
// auto serialize_time_t(const std::time_t& time) -> std::string {
//     std::stringstream transTime;
//     transTime << std::put_time(std::localtime(&time), "%F %T");
//     return transTime.str();
// }
//
// auto deserialize_time_t(const std::string& s) -> std::time_t {
//     std::tm time{};
//     time.tm_isdst = -1;
//     std::stringstream ss(s);
//     ss >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");
//     return std::mktime(&time);
// }
//
// auto todayMidnightTime() -> std::time_t {
//     std::time_t now = std::time(nullptr);
//     std::tm todayMidnight_tm = *std::localtime(&now);
//     todayMidnight_tm.tm_sec = 0;
//     todayMidnight_tm.tm_min = 0;
//     todayMidnight_tm.tm_hour = 0;
//     todayMidnight_tm.tm_mday += 1;
//     return std::mktime(&todayMidnight_tm);
// }
//
// auto advanceTimeByDays(std::time_t inputTime, float days) -> std::time_t {
//     std::tm vocActiveTime_tm = *std::localtime(&inputTime);
//     vocActiveTime_tm.tm_mday += static_cast<int>(days);
//     return std::mktime(&vocActiveTime_tm);
// }
//
// auto daysFromNow(std::time_t startTime, float intervalDays) -> int {
//     const auto sysclock_now = chrono::system_clock::from_time_t(std::time(nullptr));
//     const auto sysclock_startTime = chrono::system_clock::from_time_t(
//         advanceTimeByDays(startTime, intervalDays));
//     const auto timePoint_now = chrono::time_point_cast<chrono::days>(sysclock_now);
//     const auto timePoint_due = chrono::time_point_cast<chrono::days>(sysclock_startTime);
//     return static_cast<int>((chrono::sys_days{timePoint_due} -
//     chrono::sys_days{timePoint_now}).count());
// }
//
// auto main() -> int {
//     std::string timestr1 = "2023-07-22 14:36:51";
//     std::string timestr2 = "2023-05-22 14:36:53";
//     auto time1 = deserialize_time_t(timestr1);
//     auto time2 = deserialize_time_t(timestr2);
//     const auto chrono1 = std::chrono::system_clock::from_time_t(time1);
//     const auto chrono2 = std::chrono::system_clock::from_time_t(time2);
//     const auto tp1 = chrono::time_point_cast<chrono::days>(chrono1);
//     const auto tp2 = chrono::time_point_cast<chrono::days>(chrono2);
//     const auto diff = (chrono::sys_days{tp1} - chrono::sys_days{tp2}).count();
//     spdlog::info("Difference is: {}", diff);
//     spdlog::info("Daysa from  now {}", daysFromNow(time1, 10.F));
//     return 0;
// }
