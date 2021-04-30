#pragma once
#include <stdexcept>
// Override rapidjson assertions to throw exceptions by default
#ifndef RAPIDJSON_ASSERT
#define RAPIDJSON_ASSERT(x)                                         \
    if (!(x)) {                                                     \
        throw std::logic_error("rapidjson assertion failure: " #x); \
    }
#endif  // RAPIDJSON_ASSERT

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

template <typename JsonWrapper> class RapidMembers {
public:
    auto begin() { return json.MemberBegin(); };
    auto end() { return json.MemberEnd(); };
    RapidMembers(const JsonWrapper& _json) : json(_json){};

private:
    const JsonWrapper& json;
};
