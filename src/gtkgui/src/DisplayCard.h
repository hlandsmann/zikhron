#pragma once
#include <CardDraw.h>
#include <DataThread.h>
#include <ButtonGroup.h>
#include <NotebookPage.h>
#include <TextDraw.h>
#include <VocableList.h>
#include <annotation/Markup.h>
#include <gtkmm.h>
#include <functional>
#include <vector>

class DisplayCard : public Gtk::Box, public NotebookPage {
public:
    DisplayCard(Gtk::Overlay&);

private:
    void receive_card(DataThread::message_card& msg_card);
    void receive_annotation(DataThread::message_annotation& msg_annotation);

    void submitChoiceOfEase();
    void removeCurrentCard();
    void requestNewCard();
    void annotation_start();
    void annotation_end();
    void createControlButtons();
    Gtk::Box controlBtnBox;
    Gtk::Button btnNext;
    Gtk::Button btnReveal;
    Gtk::Separator separator1, separator2;
    Gtk::ToggleButton btnAnnotate;
    std::shared_ptr<markup::Paragraph> paragraph;
    std::shared_ptr<markup::Paragraph> annotation;
    std::vector<Ease> easeList;
    std::unique_ptr<CardDraw> cardDraw;
    std::unique_ptr<CardDraw> cardAnnotation;
    std::unique_ptr<VocableList> vocableList;

    Gtk::Overlay& overlay;

    bool displayVocabulary = false;
};
