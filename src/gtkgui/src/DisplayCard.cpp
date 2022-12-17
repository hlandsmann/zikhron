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
    createControlButtons();
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

    std::tie(paragraph, easeList, cardId) = std::move(msg_card);

    vocableList = std::make_unique<VocableList>();
    vocableList->setParagraph(paragraph, easeList);

    prepend(*vocableList);
    vocableList->set_visible(displayVocabulary);

    cardDraw = std::make_unique<CardDraw>(overlay);
    cardDraw->setParagraph(paragraph);
    cardDraw->set_visible(not btnAnnotate.property_active());
    prepend(*cardDraw);

    studyAudioFragment = CardAudioGroupDB::get().get_studyAudioFragment(cardId);
    btn_playCard.set_visible(studyAudioFragment.has_value());
    scale_mediaProgress.set_visible(studyAudioFragment.has_value());
    scale_mediaProgress.setProgress(0);
    separator1.set_visible(!studyAudioFragment.has_value());
    if (studyAudioFragment.has_value()) {
        mediaPlayer.openFile(studyAudioFragment.value().audioFile);
    }
    mediaPlayer.pause();
    controlBtnBox.set_visible(true);
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

void DisplayCard::createControlButtons() {
    controlBtnBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    controlBtnBox.append(btn_playCard);
    btn_playCard.signal_start_connect([this](MediaPlayer& _mediaPlayer) {
        double progress = scale_mediaProgress.getProgress() > 0.99 ? 0.
                                                                   : scale_mediaProgress.getProgress();
        playFromRelativeProgress(progress);
    });
    btn_playCard.signal_pause_connect([this](MediaPlayer& _mediaPlayer) { _mediaPlayer.pause(); });
    // scale_mediaProgress.set_orientation(Gtk::Orientation::HORIZONTAL);
    scale_mediaProgress.set_expand();
    scale_mediaProgress.signal_clickProgress_connect(
        [this](double progress) { playFromRelativeProgress(progress); });
    observers.push(mediaPlayer.property_timePos().observe([this](double val) {
        const StudyAudioFragment& saf = studyAudioFragment.value_or(StudyAudioFragment{});

        double relative_progress = val - saf.start;
        double length = saf.end - saf.start;
        if (length == 0)
            return;

        scale_mediaProgress.setProgress(std::clamp(relative_progress / length, 0., 1.));
    }));

    controlBtnBox.append(scale_mediaProgress);
    controlBtnBox.append(separator1);
    separator1.set_expand();
    btnReveal.set_label("Reveal Vocabulary");
    btnReveal.signal_clicked().connect([this]() {
        displayVocabulary = true;
        vocableList->set_visible(displayVocabulary);
        btnReveal.set_visible(false);
        btnNext.set_visible(true);
    });
    controlBtnBox.append(btnReveal);
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
    controlBtnBox.append(btnNext);
    controlBtnBox.append(separator2);
    separator2.set_expand();

    btnAnnotate.set_label("Annotate");
    btnAnnotate.set_valign(Gtk::Align::END);
    btnAnnotate.signal_clicked().connect([this]() {
        if (btnAnnotate.property_active())
            annotation_start();
        else
            annotation_end();
    });

    controlBtnBox.append(btnAnnotate);

    append(controlBtnBox);
    controlBtnBox.set_visible(false);
    controlBtnBox.set_valign(Gtk::Align::END);
    controlBtnBox.set_expand();
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

void DisplayCard::playFromRelativeProgress(double progress) {
    if (not studyAudioFragment.has_value())
        return;
    const StudyAudioFragment& saf = studyAudioFragment.value();
    double length = saf.end - saf.start;
    double start = saf.start + length * progress;
    mediaPlayer.play_fragment(start, saf.end);
}