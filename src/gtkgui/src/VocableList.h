#pragma once
#include <ButtonGroup.h>
#include <TextDraw.h>
#include <annotation/Ease.h>
#include <annotation/Markup.h>
#include <gtkmm.h>

class VocableList : public Gtk::Grid {
public:
    VocableList();
    void setParagraph(const std::shared_ptr<markup::Paragraph>&, const std::vector<Ease>&);
    auto getChoiceOfEase() const -> std::vector<Ease>;
    using TextDrawPtr = std::unique_ptr<TextDraw>;
    using EaseChoicePtr = std::unique_ptr<ButtonGroup>;

private:
    constexpr static int vocableFontSize = 20;
    constexpr static int vocableSpacing = 10;

    void addTextDraw(int column, int row, const std::string& markup);
    void addEaseChoice(int column, int row);
    void reset();

    std::vector<TextDrawPtr> textDrawContainer;
    std::vector<EaseChoicePtr> easeChoiceContainer;
    std::shared_ptr<markup::Paragraph> paragraph;

    int workRow = 0;
};
