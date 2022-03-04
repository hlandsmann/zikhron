#include <DisplayCard.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ranges>

namespace ranges = std::ranges;

DisplayCard::DisplayCard() {
    set_orientation(Gtk::Orientation::VERTICAL);
    set_vexpand();
    set_spacing(64);
    createCardControlButtons();
}
void DisplayCard::receive_paragraph(DataThread::message_card&& msg_paragraph) {
    std::tie(paragraph, easeList) = std::move(msg_paragraph);

    for (const auto& fragment : paragraph->getFragments())
        spdlog::info("frag: {}", fragment);

    vocableList = std::make_unique<VocableList>();
    vocableList->setParagraph(paragraph, easeList);

    prepend(*vocableList);
    vocableList->set_visible(false);

    cardDraw = std::make_unique<CardDraw>();
    cardDraw->setParagraph(paragraph);
    prepend(*cardDraw);

    cardControlBtnBox.set_visible(true);
}

void DisplayCard::createCardControlButtons() {
    cardControlBtnBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    cardControlBtnBox.append(separator1);
    separator1.set_expand();
    btnReveal.set_label("Reveal Vocabulary");
    btnReveal.signal_clicked().connect([this]() {
        vocableList->set_visible(true);
        btnReveal.set_visible(false);
        btnNext.set_visible(true);
    });
    cardControlBtnBox.append(btnReveal);
    btnReveal.set_halign(Gtk::Align::CENTER);

    btnNext.set_label("Submit choice of ease");
    btnNext.signal_clicked().connect([this]() {
        btnReveal.set_visible(true);
        btnNext.set_visible(false);
        submitChoiceOfEase();
        removeCurrentCard();
        requestNewCard();
    });
    btnNext.set_visible(false);
    cardControlBtnBox.append(btnNext);
    cardControlBtnBox.append(separator2);
    separator2.set_expand();

    btnAnnotate.set_label("Annotate");
    btnAnnotate.set_valign(Gtk::Align::END);
    btnAnnotate.signal_clicked().connect([this]() {
        if (btnAnnotate.property_active())
            annotation_start();
        else
            annotation_end();
    });

    cardControlBtnBox.append(btnAnnotate);

    append(cardControlBtnBox);
    cardControlBtnBox.set_visible(false);
    cardControlBtnBox.set_valign(Gtk::Align::END);
    cardControlBtnBox.set_expand();
}

void DisplayCard::submitChoiceOfEase() {
    auto id_ease = paragraph->getRestoredOrderOfEaseList(vocableList->getChoiceOfEase());
    if (func_submitChoiceOfEase)
        func_submitChoiceOfEase(id_ease);
}

void DisplayCard::removeCurrentCard() {
    remove(*cardDraw);
    remove(*vocableList);
}

void DisplayCard::requestNewCard() {
    if (func_requestCard)
        func_requestCard();
}

void DisplayCard::annotation_start() {
    btnNext.set_visible(false);
    btnReveal.set_visible(false);
    vocableList->set_visible(false);
    cardDraw->setModeAnnotation();
}

void DisplayCard::annotation_end() {
    btnReveal.set_visible(true);
    cardDraw->set_visible(true);
}
