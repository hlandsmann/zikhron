#include <DataThread.h>
#include <DisplayCard.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ranges>

namespace ranges = std::ranges;

DisplayCard::DisplayCard(Gtk::Overlay& ov)
    : overlay(ov), cardDraw(ov), cardAnnotation(ov), grp_single_group("single", "group") {
    set_orientation(Gtk::Orientation::VERTICAL);
    set_vexpand();
    set_spacing(64);
    append(cardDraw);
    append(cardAnnotation);
    append(vocableList);
    createControlButtons();
    DataThread::get().signal_card_connect([this](auto& msg_card) { receive_card(msg_card); });
    DataThread::get().signal_annotation_connect(
        [this](auto& msg_annotation) { receive_annotation(msg_annotation); });
    DataThread::get().requestCard();
}

void DisplayCard::receive_card(DataThread::message_card& msg_card) {
    std::tie(paragraph, easeList, cardId) = std::move(msg_card);

    vocableList.setParagraph(paragraph, easeList);

    vocableList.set_visible(displayVocabulary);

    cardDraw.setParagraph(paragraph);
    cardDraw.set_visible(not btnAnnotate.property_active());

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
    annotation = std::move(msg_annotation);
    annotation->updateAnnotationColoring();
    cardAnnotation.setParagraph(std::move(annotation));
    cardAnnotation.set_visible(btnAnnotate.property_active());
    cardAnnotation.setAnnotationMode();
};

void DisplayCard::createControlButtons() {
    controlBtnBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    controlBtnBox.set_spacing(24);
    playBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    playBox.append(btn_playCard);
    btn_playCard.signal_start_connect([this](MediaPlayer& /*_mediaPlayer*/) {
        double progress = scale_mediaProgress.getProgress() > 0.99 ? 0.
                                                                   : scale_mediaProgress.getProgress();
        playFromRelativeProgress(progress);
    });
    btn_playCard.signal_pause_connect([this](MediaPlayer& _mediaPlayer) { _mediaPlayer.pause(); });
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

    playBox.append(scale_mediaProgress);
    controlBtnBox.append(playBox);
    controlBtnBox.append(separator1);
    separator1.set_expand();

    observers.push(displayVocabulary.observe([this](bool vocableListVisible) {
        if (vocableListVisible) {
            btnNextReveal.set_label("Submit choice of ease");
        } else {
            btnNextReveal.set_label("Reveal vocabulary");
        }
        vocableList.set_visible(vocableListVisible);
    }));
    btnNextReveal.set_halign(Gtk::Align::CENTER);
    btnNextReveal.signal_clicked().connect([this]() {
        if (displayVocabulary) {
            submitChoiceOfEase();
            requestNewCard();
        }
        displayVocabulary = not displayVocabulary;
    });
    controlBtnBox.append(btnNextReveal);
    grp_single_group.setActive(0);
    controlBtnBox.append(grp_single_group);
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
    auto id_ease = paragraph->getRestoredOrderOfEaseList(vocableList.getChoiceOfEase());
    DataThread::get().submitEase(id_ease);
}

void DisplayCard::requestNewCard() { DataThread::get().requestCard(); }

void DisplayCard::annotation_start() {
    if (not annotation) {
        spdlog::error("No annotation was sent!");
        btnAnnotate.property_active() = false;
        return;
    }

    btnNextReveal.set_visible(false);
    displayVocabulary = false;
    cardDraw.set_visible(false);
    cardAnnotation.set_visible(true);
}

void DisplayCard::annotation_end() {
    btnNextReveal.set_visible(true);
    cardDraw.set_visible(true);
    vocableList.set_visible(displayVocabulary);
    cardAnnotation.set_visible(false);
}

void DisplayCard::playFromRelativeProgress(double progress) {
    if (not studyAudioFragment.has_value())
        return;
    const StudyAudioFragment& saf = studyAudioFragment.value();
    double length = saf.end - saf.start;
    double start = saf.start + length * progress;
    mediaPlayer.play_fragment(start, saf.end);
}