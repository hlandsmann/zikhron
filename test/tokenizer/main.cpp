#include <annotation/Tokenizer.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <annotation/TokenizerJpn.h>
#include <database/WordDB.h>
#include <database/WordDB_jpn.h>
#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryJpn.h>
#include <misc/Config.h>
#include <misc/Language.h>
#include <spdlog/spdlog.h>
#include <utils/ProcessPipe.h>

#include <boost/di.hpp>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
using namespace std::literals;

namespace di = boost::di;
namespace fs = std::filesystem;

static auto getProcessPipe() -> std::shared_ptr<utl::ProcessPipe>
{
    auto _ = chdir("/home/harmen/zikhron/sudachi-0.7.5/");
    return std::make_shared<utl::ProcessPipe>("/usr/bin/java", std::vector<std::string>{"-jar", "sudachi-0.7.5.jar", "-m", "C", "-a"});
}

static auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg = std::make_shared<zikhron::Config>(path_to_exe.parent_path());
    return zikhron_cfg;
}

auto main() -> int
{
    auto injector = boost::di::make_injector(
            boost::di::bind<zikhron::Config>.to(get_zikhron_cfg()),
            boost::di::bind<utl::ProcessPipe>.to(getProcessPipe()),
            boost::di::bind<annotation::Tokenizer>.to<annotation::TokenizerJpn>(),
            boost::di::bind<dictionary::Dictionary>.to<dictionary::DictionaryJpn>(),
            boost::di::bind<database::WordDB>.to<database::WordDB_jpn>(),
            boost::di::bind<Language>.to(Language::japanese));
    injector.create<std::shared_ptr<utl::ProcessPipe>>();
    const auto& tokenizer = injector.create<std::shared_ptr<annotation::TokenizerJpn>>();
    // auto _ = tokenizer->split("投降してほしけりゃてめえをあと百万体呼んで来るんだな");
    // _ = tokenizer->split("ぼ印も 押してもらいました - ぼいん？ - （小鳥） 血判の方がよかったですか？");
    auto _ = tokenizer->split("え？ ああ す… すいません なんでもありません");
    // std::string text1 = "投降してほしけりゃてめえをあと百万体呼んで来るんだな\n";
    // chdir("/home/harmen/zikhron/sudachi-0.7.5/");
    // utl::ProcessPipe proc2("/usr/bin/java", {"-jar", "/home/harmen/zikhron/sudachi-0.7.5/sudachi-0.7.5.jar", "-m", "C", "-a"});
    //
    // std::jthread myThread([&]() {
    //     proc2.write(text1);
    //     // while (true) {
    //     std::string result2 = proc2.getChunk();
    //     // if (result2.empty()) {
    //     //     break;
    //     // }
    //     spdlog::info("{}", result2);
    // });
    //
    // myThread.join();
    // }
    return 0;
}
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// Structure to hold the parsed Sudachi token data
struct SudachiToken {
    std::string surface;                // The word as it appears in text
    std::vector<std::string> pos;       // Part of Speech (comma separated in raw output)
    std::string normalized_form;        // Normalized form
    std::string dictionary_form;        // Dictionary/Base form
    std::string reading;                // Reading (usually Katakana)
    int dictionary_id;                  // Dictionary ID
    std::string extra_info;             // OOV info etc (the [] part)

    // Helper to print for debugging
    void print() const {
        std::cout << "Surface:    " << surface << "\n";
        std::cout << "POS:        ";
        for (size_t i = 0; i < pos.size(); ++i) {
            std::cout << pos[i] << (i < pos.size() - 1 ? ", " : "");
        }
        std::cout << "\n";
        std::cout << "Normalized: " << normalized_form << "\n";
        std::cout << "Reading:    " << reading << "\n";
        std::cout << "-----------------------------------\n";
    }
};

class SudachiParser {
public:
    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    static bool parseLine(const std::string& line, SudachiToken& outToken) {
        if (line.empty() || line == "EOS") return false;

        std::vector<std::string> columns = split(line, '\t');
        if (columns.size() < 6) return false;

        outToken.surface = columns[0];
        outToken.pos = split(columns[1], ',');
        outToken.normalized_form = columns[2];
        outToken.dictionary_form = columns[3];
        outToken.reading = columns[4];
        
        try { outToken.dictionary_id = std::stoi(columns[5]); } 
        catch (...) { outToken.dictionary_id = -1; }

        if (columns.size() > 6) outToken.extra_info = columns[6];
        else outToken.extra_info = "";

        return true;
    }
};

// Logic to determine if we should merge the 'current' token into the 'previous' token
bool shouldMerge(const SudachiToken& prev, const SudachiToken& curr) {
    // 1. Check if Previous is a Verb, Adjective, or already an Auxiliary (for chaining)
    bool prevIsBase = (prev.pos[0] == "動詞" || prev.pos[0] == "形容詞" || prev.pos[0] == "助動詞");

    if (!prevIsBase) return false;

    // 2. Check if Current is a "Suffix-like" element
    
    // Case A: Auxiliary Verb (助動詞) - e.g., 'reru', 'ta', 'da'
    if (curr.pos[0] == "助動詞") return true;

    // Case B: Specific Particles that function as conjugations or sentence extenders
    if (curr.pos[0] == "助詞" && curr.pos.size() > 1) {
        const std::string& subPos = curr.pos[1];

        // 1. Conjunctive Particle (接続助詞) - e.g., 'te', 'de' (呼ん-で)
        if (subPos == "接続助詞") return true;

        // 2. Nominalizing/Phrasal Particle (準体助詞) - e.g., 'n' (来る-ん-だ)
        if (subPos == "準体助詞") return true;

        // 3. Sentence-ending Particle (終助詞) - e.g., 'na', 'yo' (来るんだ-な)
        if (subPos == "終助詞") return true;
    }

    return false;
}

// int main() {
//     std::string input_data = R"(投降	名詞,普通名詞,サ変可能,*,*,*	投降	投降	トウコウ	0	[]
// し	動詞,非自立可能,*,*,サ行変格,連用形-一般	為る	する	シ	0	[]
// て	助詞,接続助詞,*,*,*,*	て	て	テ	0	[]
// ほしけりゃ	形容詞,非自立可能,*,*,形容詞,仮定形-融合	欲しい	ほしい	ホシケリャ	0	[]
// てめえ	代名詞,*,*,*,*,*	てまえ	てめえ	テメエ	0	[]
// を	助詞,格助詞,*,*,*,*	を	を	ヲ	0	[]
// あと	名詞,普通名詞,副詞可能,*,*,*	後	あと	アト	0	[]
// 百万	名詞,数詞,*,*,*,*	1000000 百万	ヒャクマン	-1	[]
// 体	名詞,普通名詞,助数詞可能,*,*,*	体	体	タイ	0	[19759]
// 呼ん	動詞,一般,*,*,五段-バ行,連用形-撥音便	呼ぶ	呼ぶ	ヨン	0	[]
// で	助詞,接続助詞,*,*,*,*	で	で	デ	0	[]
// 来る	動詞,非自立可能,*,*,カ行変格,連体形-一般	来る	来る	クル	0	[]
// ん	助詞,準体助詞,*,*,*,*	の	ん	ン	0	[]
// だ	助動詞,*,*,*,助動詞-ダ,終止形-一般	だ	だ	ダ	0	[]
// な	助詞,終助詞,*,*,*,*	な	な	ナ	0	[]
// EOS)";
//
//     std::istringstream input_stream(input_data);
//     std::string line;
//     
//     // We use a buffer to handle merging
//     std::vector<SudachiToken> finalTokens;
//     SudachiToken bufferToken;
//     bool bufferEmpty = true;
//
//     while (std::getline(input_stream, line)) {
//         SudachiToken current;
//         if (!SudachiParser::parseLine(line, current)) continue;
//
//         if (bufferEmpty) {
//             bufferToken = current;
//             bufferEmpty = false;
//         } else {
//             // Check if we should merge 'current' into 'bufferToken'
//             if (shouldMerge(bufferToken, current)) {
//                 // MERGE
//                 bufferToken.surface += current.surface;
//                 bufferToken.reading += current.reading;
//                 // Note: We keep the original POS of the head word (e.g., Verb)
//                 // because the chain acts as a single verb phrase.
//             } else {
//                 // PUSH BUFFER & START NEW
//                 finalTokens.push_back(bufferToken);
//                 bufferToken = current;
//             }
//         }
//     }
//     // Push remaining
//     if (!bufferEmpty) {
//         finalTokens.push_back(bufferToken);
//     }
//
//     std::cout << "--- Merged Results ---\n";
//     for (const auto& t : finalTokens) {
//         // Simple one-line print for overview
//         std::cout << "Word: " << std::left << std::setw(15) << t.surface 
//                   << " | POS: " << t.pos[0] 
//                   << (t.pos.size() > 1 ? "-" + t.pos[1] : "") << "\n";
//     }
//     
//     std::cout << "\n--- Detailed Check for 'Kurundana' ---\n";
//     for(const auto& t : finalTokens) {
//         if (t.surface == "来るんだな") {
//              t.print();
//         }
//     }
//
//     return 0;
// }
