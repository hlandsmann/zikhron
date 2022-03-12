#include <DataThread.h>
#include <DisplayCard.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ranges>

namespace ranges = std::ranges;

DisplayCard::DisplayCard(Gtk::Overlay& ov) : overlay(ov) {
    set_orientation(Gtk::Orientation::VERTICAL);
    set_vexpand();
    set_spacing(64);
    createCardControlButtons();
    DataThread::get().signal_card_connect([this](auto& msg_card) { receive_card(msg_card); });
    DataThread::get().signal_annotation_connect(
        [this](auto& msg_annotation) { receive_annotation(msg_annotation); });
    DataThread::get().requestCard();
}
void DisplayCard::receive_card(DataThread::message_card& msg_card) {
    if (cardDraw)
        remove(*cardDraw);
    if (vocableList)
        remove(*vocableList);

    std::tie(paragraph, easeList) = std::move(msg_card);

    vocableList = std::make_unique<VocableList>();
    vocableList->setParagraph(paragraph, easeList);

    prepend(*vocableList);
    vocableList->set_visible(displayVocabulary);

    cardDraw = std::make_unique<CardDraw>(overlay);
    cardDraw->setParagraph(paragraph);
    cardDraw->set_visible(not btnAnnotate.property_active());
    prepend(*cardDraw);

    cardControlBtnBox.set_visible(true);
}

void DisplayCard::receive_annotation(DataThread::message_annotation& msg_annotation) {
    if (cardAnnotation)
        remove(*cardAnnotation);
    annotation = std::move(msg_annotation);
    annotation->updateAnnotationColoring();
    cardAnnotation = std::make_unique<CardDraw>(overlay);
    cardAnnotation->setParagraph(std::move(annotation));
    cardAnnotation->set_visible(btnAnnotate.property_active());
    cardAnnotation->setAnnotationMode();
    prepend(*cardAnnotation);
};

void DisplayCard::createCardControlButtons() {
    cardControlBtnBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    cardControlBtnBox.append(separator1);
    separator1.set_expand();
    btnReveal.set_label("Reveal Vocabulary");
    btnReveal.signal_clicked().connect([this]() {
        displayVocabulary = true;
        vocableList->set_visible(displayVocabulary);
        btnReveal.set_visible(false);
        btnNext.set_visible(true);
    });
    cardControlBtnBox.append(btnReveal);
    btnReveal.set_halign(Gtk::Align::CENTER);

    btnNext.set_label("Submit choice of ease");
    btnNext.signal_clicked().connect([this]() {
        displayVocabulary = false;
        btnReveal.set_visible(true);
        btnNext.set_visible(false);
        submitChoiceOfEase();
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
    DataThread::get().submitEase(id_ease);
}

void DisplayCard::requestNewCard() { DataThread::get().requestCard(); }

void DisplayCard::annotation_start() {
    if (not annotation) {
        spdlog::error("No annotation was sent!");
        btnAnnotate.property_active() = false;
        return;
    }

    btnNext.set_visible(false);
    btnReveal.set_visible(false);
    vocableList->set_visible(false);
    cardDraw->set_visible(false);
    cardAnnotation->set_visible(true);
}

void DisplayCard::annotation_end() {
    btnReveal.set_visible(true);
    cardDraw->set_visible(true);
    vocableList->set_visible(displayVocabulary);
    cardAnnotation->set_visible(false);
}
