#pragma once
#include <AnnotationOverlay.h>
#include <TextDraw.h>
#include <VocableOverlay.h>
#include <annotation/Markup.h>
#include <gtkmm.h>

class CardDraw : public Gtk::Grid {
public:
    CardDraw(Gtk::Overlay&);
    void setParagraph(const std::shared_ptr<markup::Paragraph>&);
    using TextDrawPtr = std::unique_ptr<TextDraw>;
    void setAnnotationMode(bool annotation = true) { isAnnotation = annotation; };

private:
    void mouseHoverAnnotation(int textDrawIndex, int byteIndex);
    void mouseHoverStandard(int textDrawIndex, int byteIndex);
    void mouseClickAnnotation(int textDrawIndex, int byteIndex);
    void mouseClickStandard(int textDrawIndex, int byteIndex);
    constexpr static int textFontSize = 40;
    constexpr static int textSpacing = 20;
    void addTextDraw(int column, int row, const std::string& markup);
    void setupSignals();
    std::vector<TextDrawPtr> textDrawContainer;
    std::shared_ptr<markup::Paragraph> paragraph;
    std::unique_ptr<AnnotationOverlay> annotationOverlay;
    std::unique_ptr<VocableOverlay> vocableOverlay;

    Gtk::Overlay& overlay;
    bool isAnnotation = false;
};
