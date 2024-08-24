#pragma once
#include "DisplayAnnotation.h"
#include "DisplayText.h"
#include "DisplayVideo.h"
#include "DisplayVocables.h"
#include "TranslationOverlay.h"

#include <annotation/Ease.h>
#include <context/WidgetId.h>
#include <database/CardDB.h>
#include <database/CardPack.h>
#include <database/CardPackDB.h>
#include <database/TokenizationChoiceDB.h>
#include <database/Track.h>
#include <database/VideoSet.h>
#include <database/WordDB.h>
#include <misc/Identifier.h>
#include <misc/TokenizationChoice.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spaced_repetition/CardMeta.h>
#include <spaced_repetition/DataBase.h>
#include <utils/StringU8.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/MediaSlider.h>
#include <widgets/Overlay.h>
#include <widgets/ToggleButtonGroup.h>
#include <widgets/Video.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <cstddef>
#include <kocoro/kocoro.hpp>
#include <map>
#include <memory>
#include <optional>

namespace gui {

class TabCard
{
    using Align = widget::layout::Align;
    using ExpandType = widget::layout::ExpandType;
    using CardPackDB = database::CardPackDB;
    using CardPack = database::CardPack;
    using CardAudio = database::CardAudio;
    using TrackType = database::TrackType;
    enum class Proceed {
        submit,
        walkTree,
        nextTrack,
        reload,
        annotate,
    };

    enum class Mode : unsigned {
        shuffle,
        story,
    };

    enum class PlayMode : unsigned {
        stop,
        playNext,
        playUntilUnknown,
        playThrough,
    };

public:
    TabCard(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
            std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker,
            std::unique_ptr<DisplayVideo> displayVideo,
            std::unique_ptr<multimedia::MpvWrapper> mpvAudio);
    TabCard(const TabCard&) = delete;
    TabCard(TabCard&&) = delete;
    auto operator=(const TabCard&) -> TabCard& = delete;
    auto operator=(TabCard&&) -> TabCard& = delete;
    virtual ~TabCard() = default;

    void setUp(widget::Layer& layer);
    void displayOnLayer(widget::Layer& layer);
    void slot_playVideoSet(database::VideoSetPtr);

private:
    using VocableId_Ease = std::map<VocableId, Ease>;
    auto videoPlaybackTask() -> kocoro::Task<>;
    auto feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>;
    auto annotationTask(sr::CardMeta& cardMeta,
                        const std::shared_ptr<database::CardDB>& cardDB,
                        std::shared_ptr<widget::Layer> cardLayer) -> kocoro::Task<bool>;
    void clearStudy(const std::shared_ptr<widget::Layer>& cardLayer,
                    const std::shared_ptr<widget::Layer>& vocableLayer);
    void prepareStudy(sr::CardMeta& cardMeta,
                      std::shared_ptr<database::WordDB> wordDB,
                      const std::shared_ptr<widget::Layer>& cardLayer,
                      const std::shared_ptr<widget::Layer>& vocableLayer);
    void loadTrack();

    void setupCardWindow(widget::Window& cardWindow);
    void doCardWindow(widget::Window& cardWindow);

    void setupCtrlWindow(widget::Window& ctrlWindow);
    void doCtrlWindow(widget::Window& ctrlWindow);

    static void setupAudioCtrlBox(widget::Box& ctrlBox);
    void doAudioCtrlBox(widget::Box& ctrlBox);
    static void setupVideoCtrlBox(widget::Box& ctrlBox);
    void doVideoCtrlBox(widget::Box& ctrlBox);

    static void setupSecondaryCtrl(widget::Layer& ctrlLayer);
    void doSecondaryCtrl(widget::Layer& ctrlLayer);

    void handlePlayback(widget::ImageButton& btnPlay, widget::MediaSlider& sliderProgress);
    void handleCardSubmission(widget::Button& btnReveal,
                              widget::Button& btnSubmit,
                              widget::Button& btnNext);
    void handleMode(widget::ToggleButtonGroup& tbgMode);
    void handlePlayMode(widget::ImageButton& btnPlayMode);
    void handleSubAddCut(widget::ImageButton& btnCutPrev,
                         widget::ImageButton& btnAddPrev,
                         widget::ImageButton& btnAddNext,
                         widget::ImageButton& btnCutNext);
    void handleSelection(widget::ImageButton& btnSelect,
                         widget::ImageButton& btnUnselect);
    void handleNextPreviousVideo(widget::ImageButton& btnContinue,
                                 widget::ImageButton& btnPrevious,
                                 widget::ImageButton& btnNext,
                                 widget::ImageButton& btnOpenSegment);
    void handleNextPrevious(widget::ImageButton& btnFirst,
                            widget::ImageButton& btnPrevious,
                            widget::ImageButton& btnNext,
                            widget::ImageButton& btnLast);
    void handleToggleTimeSelection(widget::ImageButton& btnTimeSubtitle);
    void handleTimeSelection(widget::ImageButton& btnTimeDelFront,
                            widget::ImageButton& btnTimeAddFront,
                            widget::ImageButton& btnTimeAddBack,
                            widget::ImageButton& btnTimeDelBack);
    void handleTranslation(widget::ImageButton& btnTranslation);
    void handleAnnotate(widget::ImageButton& btnAnnotate);
    void handleDataBaseSave(widget::ImageButton& btnSave);

    std::shared_ptr<sr::DataBase> dataBase;
    std::shared_ptr<kocoro::SynchronousExecutor> executor;
    using BoxPtr = std::shared_ptr<widget::Box>;
    std::shared_ptr<kocoro::PersistentSignal<BoxPtr>> signalCardBox;
    std::shared_ptr<kocoro::VolatileSignal<Proceed>> signalProceed;
    std::shared_ptr<kocoro::VolatileSignal<TokenizationChoice>> signalAnnotationDone;

    std::shared_ptr<widget::Layer> secondaryCtrlLayer;
    std::unique_ptr<DisplayText> displayText;
    std::unique_ptr<DisplayAnnotation> displayAnnotation;
    std::unique_ptr<DisplayVocables> displayVocables;
    std::optional<database::Track> track;

    std::unique_ptr<DisplayVideo> displayVideo;
    std::unique_ptr<TranslationOverlay> translationOverlay;

    std::shared_ptr<widget::Overlay> overlayForVocable;
    std::shared_ptr<widget::Overlay> overlayForAnnotation;
    std::shared_ptr<widget::Overlay> overlayForTranslation;
    std::shared_ptr<widget::Video> video;

    context::WidgetId boxId{};

    bool revealVocables{false};
    Mode mode{Mode::shuffle};
    PlayMode playMode{PlayMode::playNext};

    std::shared_ptr<multimedia::MpvWrapper> mpvAudio;
    std::shared_ptr<multimedia::MpvWrapper> mpvVideo;
};
} // namespace gui
