#pragma once
#include <ButtonGroup.h>
#include <CardDraw.h>
#include <DataThread.h>
#include <MediaButtons.h>
#include <MediaSlider.h>
#include <NotebookPage.h>
#include <TextDraw.h>
#include <VocableList.h>
#include <annotation/Markup.h>
#include <gtkmm.h>
#include <multimedia/CardAudioGroup.h>
#include <multimedia/MediaPlayer.h>
#include <utils/Property.h>
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
    void playFromRelativeProgress(double progress);
    utl::ObserverCollection observers;
    Gtk::Overlay& overlay;

    MediaPlayer mediaPlayer;
    PlayPauseButton btn_playCard{mediaPlayer};
    MediaSlider scale_mediaProgress;
    std::optional<StudyAudioFragment> studyAudioFragment;

    Gtk::Box controlBtnBox;
    Gtk::Button btnNextReveal;
    utl::Property<bool> vocablelistVisible = false;
    Gtk::Separator separator1, separator2;
    Gtk::ToggleButton btnAnnotate;
    std::shared_ptr<markup::Paragraph> paragraph;
    std::shared_ptr<markup::Paragraph> annotation;
    std::vector<Ease> easeList;
    CardDraw cardDraw;
    CardDraw cardAnnotation;
    VocableList vocableList;
    uint cardId = 0;

    utl::Property<bool> displayVocabulary = false;
};
