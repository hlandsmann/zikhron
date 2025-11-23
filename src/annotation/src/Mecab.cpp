#include "Mecab.h"

#include "MecabWrapper.h"

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace annotation {

Mecab::Mecab()
    : mecabWrapper{std::make_shared<MecabWrapper>()}
    , log{std::make_unique<spdlog::logger>("", std::make_shared<spdlog::sinks::null_sink_mt>())}
{}

auto Mecab::split(const std::string& text) -> std::vector<MecabToken>
{
    return mecabWrapper->split(text);
}

void Mecab::setDebugSink(spdlog::sink_ptr sink)
{
    log = std::make_unique<spdlog::logger>("", sink);
    mecabWrapper->setDebugSink(sink);
}

// Type and POS fields in unidic-cwj-202302
//
//  type,pos1,pos2,pos3,pos4
const std::map<std::array<std::string, 5>, POS_Japanese> posMap = {
        {{"人名", "名詞", "固有名詞", "人名", "一般"}, POS_Japanese::verb},
        {{"他", "感動詞", "フィラー", "", ""}, POS_Japanese::verb},
        {{"他", "感動詞", "一般", "", ""}, POS_Japanese::verb},
        {{"他", "接続詞", "", "", ""}, POS_Japanese::verb},
        {{"体", "代名詞", "", "", ""}, POS_Japanese::verb},
        {{"体", "名詞", "助動詞語幹", "", ""}, POS_Japanese::verb},
        {{"体", "名詞", "普通名詞", "サ変可能", ""}, POS_Japanese::verb},
        {{"体", "名詞", "普通名詞", "サ変形状詞可能", ""}, POS_Japanese::verb},
        {{"体", "名詞", "普通名詞", "一般", ""}, POS_Japanese::verb},
        {{"体", "名詞", "普通名詞", "副詞可能", ""}, POS_Japanese::verb},
        {{"体", "名詞", "普通名詞", "助数詞可能", ""}, POS_Japanese::verb},
        {{"体", "名詞", "普通名詞", "形状詞可能", ""}, POS_Japanese::verb},
        {{"係助", "助詞", "係助詞", "", ""}, POS_Japanese::verb},
        {{"副助", "助詞", "副助詞", "", ""}, POS_Japanese::verb},
        {{"助動", "助動詞", "", "", ""}, POS_Japanese::verb},
        {{"助動", "形状詞", "助動詞語幹", "", ""}, POS_Japanese::verb},
        {{"助数", "接尾辞", "名詞的", "助数詞", ""}, POS_Japanese::verb},
        {{"名", "名詞", "固有名詞", "人名", "名"}, POS_Japanese::verb},
        {{"固有名", "名詞", "固有名詞", "一般", ""}, POS_Japanese::verb},
        {{"国", "名詞", "固有名詞", "地名", "国"}, POS_Japanese::verb},
        {{"地名", "名詞", "固有名詞", "地名", "一般"}, POS_Japanese::verb},
        {{"姓", "名詞", "固有名詞", "人名", "姓"}, POS_Japanese::verb},
        {{"接助", "助詞", "接続助詞", "", ""}, POS_Japanese::verb},
        {{"接尾体", "接尾辞", "名詞的", "サ変可能", ""}, POS_Japanese::verb},
        {{"接尾体", "接尾辞", "名詞的", "一般", ""}, POS_Japanese::verb},
        {{"接尾体", "接尾辞", "名詞的", "副詞可能", ""}, POS_Japanese::verb},
        {{"接尾用", "接尾辞", "動詞的", "", ""}, POS_Japanese::verb},
        {{"接尾相", "接尾辞", "形容詞的", "", ""}, POS_Japanese::verb},
        {{"接尾相", "接尾辞", "形状詞的", "", ""}, POS_Japanese::verb},
        {{"接頭", "接頭辞", "", "", ""}, POS_Japanese::verb},
        {{"数", "名詞", "数詞", "", ""}, POS_Japanese::verb},
        {{"格助", "助詞", "格助詞", "", ""}, POS_Japanese::verb},
        {{"準助", "助詞", "準体助詞", "", ""}, POS_Japanese::verb},
        {{"用", "動詞", "一般", "", ""}, POS_Japanese::verb},
        {{"用", "動詞", "非自立可能", "", ""}, POS_Japanese::verb},
        {{"相", "副詞", "", "", ""}, POS_Japanese::verb},
        {{"相", "形容詞", "一般", "", ""}, POS_Japanese::verb},
        {{"相", "形容詞", "非自立可能", "", ""}, POS_Japanese::verb},
        {{"相", "形状詞", "タリ", "", ""}, POS_Japanese::verb},
        {{"相", "形状詞", "一般", "", ""}, POS_Japanese::verb},
        {{"相", "連体詞", "", "", ""}, POS_Japanese::verb},
        {{"終助", "助詞", "終助詞", "", ""}, POS_Japanese::verb},
        {{"補助", "空白", "", "", ""}, POS_Japanese::verb},
        {{"補助", "補助記号", "一般", "", ""}, POS_Japanese::verb},
        {{"補助", "補助記号", "句点", "", ""}, POS_Japanese::verb},
        {{"補助", "補助記号", "括弧閉", "", ""}, POS_Japanese::verb},
        {{"補助", "補助記号", "括弧開", "", ""}, POS_Japanese::verb},
        {{"補助", "補助記号", "読点", "", ""}, POS_Japanese::verb},
        {{"補助", "補助記号", "ＡＡ", "一般", ""}, POS_Japanese::verb},
        {{"補助", "補助記号", "ＡＡ", "顔文字", ""}, POS_Japanese::verb},
        {{"記号", "記号", "一般", "", ""}, POS_Japanese::verb},
        {{"記号", "記号", "文字", "", ""}, POS_Japanese::verb}};

//      </pre>
//  </details>
} // namespace annotation
