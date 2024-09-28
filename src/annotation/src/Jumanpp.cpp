// oriented on juman_format.cc
#include "Jumanpp.h"

#include "detail/JumanppWrapper.h"

#pragma GCC diagnostic push
#include <core/analysis/analysis_result.h>
#include <core/analysis/lattice_config.h>
#include <core/analysis/output.h>
#include <core/analysis/rnn_scorer.h>
#include <jumandic/shared/juman_format.h>
#include <jumandic/shared/jumandic_env.h>
#include <jumandic/shared/jumandic_id_resolver.h>
#include <jumandic/shared/jumanpp_args.h>
#pragma GCC diagnostic pop

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <util/status.hpp>
#include <vector>

namespace jumanpp {
// namespace jumandic = jumanpp::jumandic;
// namespace core = jumanpp::core;

template<class Fun>
void tryThrow(Fun fun, std::string err)
{
    Status status = fun();
    if (status.isOk()) {
        return;
    }
    std::stringstream ss;
    ss << err << ": " << status;
    throw std::runtime_error(ss.str());
}

auto JumandicFields::initialize(const core::analysis::OutputManager& om) -> Status
{
    JPP_RETURN_IF_ERROR(om.stringField("surface", &surface));
    JPP_RETURN_IF_ERROR(om.stringField("pos", &pos));
    JPP_RETURN_IF_ERROR(om.stringField("subpos", &subpos));
    JPP_RETURN_IF_ERROR(om.stringField("conjtype", &conjType));
    JPP_RETURN_IF_ERROR(om.stringField("conjform", &conjForm));
    JPP_RETURN_IF_ERROR(om.stringField("baseform", &baseform));
    JPP_RETURN_IF_ERROR(om.stringField("reading", &reading));
    JPP_RETURN_IF_ERROR(om.stringField("canonic", &canonicForm));
    JPP_RETURN_IF_ERROR(om.kvListField("features", &features));
    return Status::Ok();
}

Jumanpp::Jumanpp()
    : exec{getConfiguration()}
{
    initExecutor();
    const auto& outputManager = exec.analyzerPtr()->output();
    // tryThrow([&] { return idResolver.initialize(outputManager.dic()); }, "idResolver.initialize");
    tryThrow([&] { return fields.initialize(outputManager); }, "fields.initialize");
}

auto Jumanpp::tokenize(const std::string& text) -> std::vector<annotation::JumanppToken>
{
    // auto fmt = std::make_unique<jumandic::output::JumanFormat>();
    // auto s = fmt->initialize(exec.analyzerPtr()->output());
    // s = exec.analyzerPtr()->analyze(text);
    // s = fmt->format(*exec.analyzerPtr(), "");
    // std::cout << "res: " << fmt->result();
    std::vector<annotation::JumanppToken> result;
    auto analysisResult = core::analysis::AnalysisResult{};
    core::analysis::AnalysisPath top1;
    core::analysis::NodeWalker walker;

    auto* analyzer = exec.analyzerPtr();
    const auto& outputManager = analyzer->output();
    tryThrow([&] { return analyzer->analyze(text); }, "analyzer->analyze(text)");
    tryThrow([&] { return analysisResult.reset(*analyzer); }, "analysisResult.reset(*analyzer)");
    tryThrow([&] { return analysisResult.fillTop1(&top1); }, "analysisResult.fillTop1(&top1)");
    while (top1.nextBoundary()) {
        if (top1.remainingNodesInChunk() <= 0) {
            throw std::runtime_error("no nodes in chunk");
        }
        core::analysis::ConnectionPtr connection{};
        if (!top1.nextNode(&connection)) {
            throw std::runtime_error("failed to load a node");
        }

        bool next = true;
        while (next) {
            core::analysis::LatticeNodePtr nodePtr{.boundary = connection.boundary, .position = connection.right};
            outputManager.locate(nodePtr, &walker);
            while (walker.next()) {
                // std::cout << fields.surface[walker]
                //           << " " << fields.baseform[walker]
                //           << " " << fields.canonicForm[walker]
                //           << " pos: " << fields.pos[walker]
                //           << " spos: " << fields.subpos[walker]
                //           << "\n";
                result.push_back(annotation::JumanppToken{
                        .surface = fields.surface[walker].str(),
                        .pos = fields.pos[walker].str(),
                        .subpos = fields.subpos[walker].str(),
                        .conjType = fields.conjType[walker].str(),
                        .conjForm = fields.conjForm[walker].str(),
                        .baseform = fields.baseform[walker].str(),
                        .reading = fields.reading[walker].str(),
                        .canonicForm = fields.canonicForm[walker].str(),
                });
            }
            next = top1.nextNode(&connection);
        }
    }
    return result;
}

auto Jumanpp::getConfiguration() -> jumanpp::jumandic::JumanppConf
{
    jumanpp::jumandic::JumanppConf conf;
    jumanpp::core::analysis::rnn::RnnInferenceConfig rnnConfig{};
    conf.configFile = "/home/harmen/src/zikhron/third_party/jumanpp/model/jumandic.conf";
    conf.modelFile = "/home/harmen/src/zikhron/third_party/jumanpp/model/jumandic.jppmdl";
    rnnConfig.nceBias = 5.62844F;
    rnnConfig.unkConstantTerm = -3.47481F;
    rnnConfig.unkLengthPenalty = -2.92994951022F;
    rnnConfig.perceptronWeight = 1.F;
    rnnConfig.rnnWeight = 0.0176F;
    conf.rnnConfig = rnnConfig;

    return conf;
}

void Jumanpp::initExecutor()
{
    auto s = exec.init();
    if (!s.isOk()) {
        std::stringstream ss;
        ss << "failed to load model from disk: " << s;
        throw std::runtime_error(ss.str());
    }
}
} // namespace jumanpp
