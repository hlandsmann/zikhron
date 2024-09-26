#pragma once
#include "JpnTokenizer.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic ignored "-Wtype-limits"
#include <core/analysis/output.h>
#include <jumandic/shared/jumandic_env.h>
#include <jumandic/shared/jumandic_id_resolver.h>
#include <jumandic/shared/jumanpp_args.h>
#pragma GCC diagnostic pop

#include <string>
#include <util/status.hpp>

namespace jumanpp {

struct JumandicFields
{
    core::analysis::StringField surface;
    core::analysis::StringField pos;
    core::analysis::StringField subpos;
    core::analysis::StringField conjType;
    core::analysis::StringField conjForm;
    core::analysis::StringField baseform;
    core::analysis::StringField reading;
    core::analysis::StringField canonicForm;
    core::analysis::KVListField features;

    auto initialize(const core::analysis::OutputManager& om) -> Status;
};

class Jumanpp
{
public:
    Jumanpp();

    void tokenize(const std::string& text);

private:
    [[nodiscard]] static auto getConfiguration() -> jumanpp::jumandic::JumanppConf;
    void initExecutor();

    jumandic::JumanppExec exec;
    // jumandic::JumandicIdResolver idResolver;
    JumandicFields fields;
};
} // namespace jumanpp
