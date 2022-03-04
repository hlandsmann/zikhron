#pragma once
#include <TextDraw.h>
#include <annotation/Markup.h>
#include <gtkmm.h>

class CardDraw : public Gtk::Grid {
public:
    CardDraw();
    void setParagraph(const std::shared_ptr<markup::Paragraph>&);
    using TextDrawPtr = std::unique_ptr<TextDraw>;
    void setModeAnnotation(bool annotation = true);

private:
    constexpr static int textFontSize = 40;
    constexpr static int textSpacing = 20;
    void addTextDraw(int column, int row, const std::string& markup);
    void setupSignals();
    std::vector<TextDrawPtr> textDrawContainer;
    std::shared_ptr<markup::Paragraph> paragraph;
};
