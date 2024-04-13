#pragma once
#include <ButtonGroup.h>
#include <TextDraw.h>
#include <annotation/Ease.h>
#include <annotation/Markup.h>
#include <gtkmm.h>

#include <memory>
#include <string>
#include <vector>

class VocableList : public Gtk::Grid
{
public:
    VocableList();
    void setParagraph(const std::shared_ptr<markup::Paragraph>&, const std::vector<Ease>&);
    auto getChoiceOfEase() -> std::vector<Ease>;
    using TextDrawPtr = std::unique_ptr<TextDraw>;
    using EaseChoicePtr = std::unique_ptr<ButtonGroup>;
    using LabelPtr = std::unique_ptr<Gtk::Label>;

private:
    constexpr static int vocableFontSize = 20;
    constexpr static int vocableSpacing = 10;

    void addTextDraw(int column, int row, const std::string& markup);
    void addEaseChoice(int column, int row);
    void addLabel(int column, int row);
    void reset();

    std::vector<TextDrawPtr> textDrawContainer;
    std::vector<EaseChoicePtr> easeChoiceContainer;
    std::vector<LabelPtr> labelContainer;
    std::shared_ptr<markup::Paragraph> paragraph;
    std::vector<Ease> easeList;
    int workRow = 0;
};
