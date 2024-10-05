#pragma once
#include "Token.h"

#include <memory>
#include <string>
#include <vector>

namespace annotation {
class MecabWrapper;

struct MecabToken
{
    std::string surface;
    std::string pos1;
    std::string pos2;
    std::string pos3;
    std::string pos4;
    std::string conjugationType;
    std::string conjugationShape;
    std::string lemmaReading;
    std::string lemma;
    std::string orth;
    std::string pronounciation;
    std::string orthBase;
    std::string pronounciationBase;
    std::string goshu;
    std::string iType;
    std::string iForm;
    std::string fType;
    std::string fForm;
    std::string iConType;
    std::string fConType;
    std::string lemmaType;
};

class Mecab
{
public:
    Mecab();
    auto split(const std::string& text) -> std::vector<MecabToken>;

private:
    std::shared_ptr<MecabWrapper> mecabWrapper;
};
} // namespace annotation
