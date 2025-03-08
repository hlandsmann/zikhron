#include <database/SpacedRepetitionData.h>
#include <database/TokenizationChoiceDB.h>
#include <database/TokenizationChoiceDbChi.h>
#include <database/WordDB.h>
#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <dictionary/DictionaryJpn.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spaced_repetition/Scheduler.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <utils/format.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <ratio>
#include <string>
#include <utility>

namespace fs = std::filesystem;

auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    return std::make_shared<zikhron::Config>(path_to_exe.parent_path());
}


auto main() -> int
{
    using Rating = sr::Rating;
    auto scheduler = sr::Scheduler{};
    auto review = [&scheduler](const database::SpacedRepetitionData& srd, Rating rating) -> database::SpacedRepetitionData {
        database::SpacedRepetitionData srdTemp = srd;
        using Days = std::chrono::duration<double, std::ratio<86400>>;
        // auto dur = scheduler.getIntervalDays(srd);
        auto dur = srd.due - srd.reviewed;
        srdTemp.reviewed = srd.reviewed - dur + duration_cast<std::chrono::nanoseconds>(Days{srd.shiftBackward});
        srdTemp.due = std::chrono::system_clock::now();

        auto srsFail = scheduler.review(srdTemp, Rating::fail);
        auto srsPass = scheduler.review(srdTemp, Rating::pass);
        // auto srsEasy = scheduler.review(srd, Rating::familiar);
        spdlog::info("incd: {}, ----> {}", srdTemp.getDueInTimeLabel(), srdTemp.serialize());
        spdlog::info("fail: {}, ----> {}", srsFail.getDueInTimeLabel(), srsFail.serialize());
        spdlog::info("pass: {}, ----> {}", srsPass.getDueInTimeLabel(), srsPass.serialize());
        // spdlog::info("easy: {}", srsEasy.getDueInTimeLabel());
        switch (rating) {
        case Rating::fail:
            spdlog::info("Fail");
            return srsFail;
        case Rating::pass:
            spdlog::info("Pass");
            return srsPass;
            case Rating::familiar:
                spdlog::info("Familiar");
                return srsPass;
        }
        std::unreachable();
    };
    // std::string ser = "2025-02-27 19:28:08,2025-03-02 01:00:00,0,0,1.0000,1.5850,review,true,1018,0,1,";
    std::string ser = "2025-03-11 07:39:36,2025-03-11 07:49:36,200,200,1.2000,0.9670,relearning,true,683,653,0,1,";
    // database::SpacedRepetitionData srd{};
    auto srd = database::SpacedRepetitionData::deserialize(ser);
    srd = review(srd, Rating::fail);
    srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::fail);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::fail);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::fail);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::fail);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::pass);
    // // srd = review(srd, Rating::fail);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::fail);
    // srd = review(srd, Rating::pass);
    // // srd = review(srd, Rating::fail);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::pass);
    // srd = review(srd, Rating::pass);

    spdlog::info("Time: {}", srd.getDueInTimeLabel());
}

