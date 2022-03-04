#pragma once
#include <CardDraw.h>
#include <DataThread.h>
#include <EaseChoice.h>
#include <TextDraw.h>
#include <VocableList.h>
#include <annotation/Markup.h>
#include <gtkmm.h>
#include <functional>
#include <vector>

class DisplayCard : public Gtk::Box {
public:
    DisplayCard();
    void receive_paragraph(DataThread::message_card&& msg_paragraph);
    void signal_submitEase(const std::function<void(const VocabularySR::Id_Ease_vt&)>& functor) {
        func_submitChoiceOfEase = functor;
    };
    void signal_requestCard(const std::function<void()>& functor) { func_requestCard = functor; };
private:
    void submitChoiceOfEase();
    void removeCurrentCard();
    void requestNewCard();
    void annotation_start();
    void annotation_end();
    void createCardControlButtons();
    Gtk::Box cardControlBtnBox;
    Gtk::Button btnNext;
    Gtk::Button btnReveal;
    Gtk::Separator separator1, separator2;
    Gtk::ToggleButton btnAnnotate;
    std::shared_ptr<markup::Paragraph> paragraph;
    std::vector<Ease> easeList;
    std::unique_ptr<CardDraw> cardDraw;
    std::unique_ptr<VocableList> vocableList;

    std::function<void(const VocabularySR::Id_Ease_vt&)> func_submitChoiceOfEase;
    std::function<void()> func_requestCard;
};
