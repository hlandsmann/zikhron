
#include <annotation/TokenizerJpn.h>
#include <dictionary/JpnDictionary.h>
#include <misc/Config.h>

#include <filesystem>
// #include "Jumanpp.h"

// #include <core/analysis/rnn_scorer.h>
// #include <jumandic/shared/juman_format.h>
// #include <jumandic/shared/jumandic_env.h>
// #include <jumandic/shared/jumanpp_args.h>

#include <iostream>
#include <memory>
#include <string>

// namespace jumandic = jumanpp::jumandic;

namespace fs = std::filesystem;

static auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg = std::make_shared<zikhron::Config>(path_to_exe.parent_path());
    return zikhron_cfg;
}

auto main() -> int
{
    std::string text1 = "投降してほしけりゃてめえをあと百万体呼んで来るんだな";
    std::string text2 = "だから全然 心洗われるように 聞こえんぞ 千斗いすずよ";
    std::string text3 = "だから全然心洗われるように聞こえんぞ千斗いすずよ";

    // jumandic::JumanppConf conf;
    // jumanpp::core::analysis::rnn::RnnInferenceConfig rnnConfig{};
    // conf.configFile = "/home/harmen/src/zikhron/third_party/jumanpp/model/jumandic.conf";
    // conf.modelFile = "/home/harmen/src/zikhron/third_party/jumanpp/model/jumandic.jppmdl";
    // rnnConfig.nceBias = 5.62844F;
    // rnnConfig.unkConstantTerm = -3.47481;
    // rnnConfig.unkLengthPenalty = -2.92994951022F;
    // rnnConfig.perceptronWeight = 1.F;
    // rnnConfig.rnnWeight = 0.0176;
    // conf.rnnConfig = rnnConfig;
    // // jumandic::JumanppConf cmdline;
    // // conf.mergeWith(cmdline);
    //
    // jumandic::JumanppExec exec{conf};
    // auto s = exec.init();
    // if (!s.isOk()) {
    //     if (conf.outputType == jumandic::OutputType::Version) {
    //         exec.printFullVersion();
    //         return 0;
    //     }
    //
    //     if (conf.modelFile.isDefault()) {
    //         std::cerr << "Model file was not specified\n";
    //         return 1;
    //     }
    //
    //     if (conf.outputType == jumandic::OutputType::ModelInfo) {
    //         exec.printModelInfo();
    //         return 0;
    //     }
    //
    //     std::cerr << "failed to load model from disk: " << s;
    //     return 1;
    // }
    //
    // s = exec.analyzerPtr()->analyze(text1);
    // auto fmt = std::make_unique<jumandic::output::JumanFormat>();
    // s = fmt->initialize(exec.analyzerPtr()->output());
    // if (!s) {
    //     std::cout << "nOK\n";
    // }
    // // auto fmt = exec.format();
    // s = fmt->format(*exec.analyzerPtr(), "");
    // if (!s) {
    //     std::cout << "nOK\n";
    // }
    //
    // // std::cout << "res: " << exec.format()->result();
    // std::cout << text1 << "\n";
    // std::cout << "res: " << fmt->result();
    //
    // s = exec.analyzerPtr()->analyze(text2);
    // s = fmt->format(*exec.analyzerPtr(), "");
    // std::cout << text2 << "\n";
    // std::cout << "res: " << fmt->result();
    //
    // s = exec.analyzerPtr()->analyze(text3);
    // s = fmt->format(*exec.analyzerPtr(), "");
    // std::cout << text3 << "\n";
    // std::cout << "res: " << fmt->result();
    //
    auto jpnDictionary = std::make_shared<dictionary::JpnDictionary>(get_zikhron_cfg());
    auto jpnTokenizer = std::make_unique<annotation::JpnTokenizer>(jpnDictionary);
    jpnTokenizer->split(text1);
    // auto jumanxx = jumanpp::Jumanpp{};
    // jumanxx.tokenize(text1);
}
