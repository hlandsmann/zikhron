
#include <core/analysis/rnn_scorer.h>
#include <jumandic/shared/jumandic_env.h>
#include <jumandic/shared/jumanpp_args.h>

#include <iostream>

namespace jumandic = jumanpp::jumandic;

auto main() -> int
{
    jumandic::JumanppConf conf;
    jumanpp::core::analysis::rnn::RnnInferenceConfig rnnConfig{};
    conf.configFile = "/home/harmen/src/zikhron/third_party/jumanpp/model/jumandic.conf";
    conf.modelFile = "/home/harmen/src/zikhron/third_party/jumanpp/model/jumandic.jppmdl";
    rnnConfig.nceBias = 5.62844F;
    rnnConfig.unkConstantTerm = -3.47481;
    rnnConfig.unkLengthPenalty = -2.92994951022F;
    rnnConfig.perceptronWeight = 1.F;
    rnnConfig.rnnWeight = 0.0176;
    conf.rnnConfig = rnnConfig;
    // jumandic::JumanppConf cmdline;
    // conf.mergeWith(cmdline);

    jumandic::JumanppExec exec{conf};
    auto s = exec.init();
    if (!s.isOk()) {
        if (conf.outputType == jumandic::OutputType::Version) {
            exec.printFullVersion();
            return 0;
        }

        if (conf.modelFile.isDefault()) {
            std::cerr << "Model file was not specified\n";
            return 1;
        }

        if (conf.outputType == jumandic::OutputType::ModelInfo) {
            exec.printModelInfo();
            return 0;
        }

        std::cerr << "failed to load model from disk: " << s;
        return 1;
    }

    std::string text = "投降してほしけりゃてめえをあと百万体呼んで来るんだな";
    s = exec.analyzerPtr()->analyze(text);
    auto fmt = exec.format();
    s = fmt->format(*exec.analyzerPtr(), "");
    if (!s) {
        std::cout << "nOK\n";
    }

    std::cout << "res: " << exec.format()->result();
}
