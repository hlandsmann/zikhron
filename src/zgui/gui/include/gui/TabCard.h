#pragma once
#include "DisplayAnnotation.h"
#include "DisplayText.h"
#include "DisplayVocables.h"

#include <annotation/AnnotationFwd.h>
#include <annotation/Ease.h>
#include <annotation/TokenizationChoiceDB.h>
#include <card_data_base/CardPack.h>
#include <card_data_base/CardPackDB.h>
#include <context/WidgetId.h>
#include <misc/Identifier.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spaced_repetition/DataBase.h>
#include <utils/StringU8.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/MediaSlider.h>
#include <widgets/Overlay.h>
#include <widgets/ToggleButtonGroup.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <cstddef>
#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <map>
#include <memory>
#include <optional>

namespace gui {

class TabCard
{
    using Align = widget::layout::Align;
    using ExpandType = widget::layout::ExpandType;
    using CardPackDB = annotation::CardPackDB;
    using CardPack = annotation::CardPack;
    enum class Proceed {
        submit_walkTree,
        submit_next,
        first,
        previous,
        next,
        last,
        reload,
        annotate,
    };

    enum class Mode : std::size_t {
        shuffle,
        story,
    };

    class CardAudioInfo
    {
    public:
        CardAudioInfo(CardId cardId, const annotation::CardPackDB& cardPackDB);
        CardAudioInfo() = default;
        [[nodiscard]] auto firstId() const -> std::optional<CardId>;
        [[nodiscard]] auto previousId() const -> std::optional<CardId>;
        [[nodiscard]] auto nextId() const -> std::optional<CardId>;
        [[nodiscard]] auto lastId() const -> std::optional<CardId>;
        [[nodiscard]] auto getAudio() const -> std::optional<std::filesystem::path>;
        [[nodiscard]] auto getStartTime() const -> double;
        [[nodiscard]] auto getEndTime() const -> double;

    private:
        annotation::CardAudio cardAudio;
        std::shared_ptr<CardPack> cardPack;
    };

public:
    TabCard(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
            std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker,
            std::unique_ptr<multimedia::MpvWrapper> mpv);
    TabCard(const TabCard&) = delete;
    TabCard(TabCard&&) = delete;
    auto operator=(const TabCard&) -> TabCard& = delete;
    auto operator=(TabCard&&) -> TabCard& = delete;
    virtual ~TabCard() = default;

    void setUp(std::shared_ptr<widget::Layer> layer);
    void displayOnLayer(widget::Layer& layer);

private:
    using VocableId_Ease = std::map<VocableId, Ease>;
    auto feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>;

    void setupCardWindow(widget::Window& cardWindow);
    void doCardWindow(widget::Window& cardWindow);

    static void setupCtrlWindow(widget::Window& ctrlWindow);
    void doCtrlWindow(widget::Window& ctrlWindow);

    void handlePlayback(widget::ImageButton& btnPlay, widget::MediaSlider& sliderProgress);
    void handleCardSubmission(widget::Button& btnReveal, widget::Button& btnSubmit, widget::Button& btnNext);
    void handleMode(widget::ToggleButtonGroup& tbgMode);
    void handleNextPrevious(widget::ImageButton& btnFirst,
                            widget::ImageButton& btnPrevious,
                            widget::ImageButton& btnFollowing,
                            widget::ImageButton& btnLast);
    void handleAnnotate(widget::ImageButton& btnAnnotate);
    void handleDataBaseSave(widget::ImageButton& btnSave);

    std::shared_ptr<sr::DataBase> dataBase;
    std::shared_ptr<kocoro::SynchronousExecutor> executor;

    std::unique_ptr<DisplayText> displayText;
    std::unique_ptr<DisplayAnnotation> displayAnnotation;
    std::unique_ptr<DisplayVocables> displayVocables;
    std::unique_ptr<CardAudioInfo> cardAudioInfo;

    std::shared_ptr<widget::Overlay> overlay;

    using BoxPtr = std::shared_ptr<widget::Box>;
    std::shared_ptr<kocoro::PersistentSignal<BoxPtr>> signalCardBox;
    std::shared_ptr<kocoro::VolatileSignal<Proceed>> signalProceed;
    using TokenizationChoice = annotation::TokenizationChoice;
    std::shared_ptr<kocoro::VolatileSignal<TokenizationChoice>> signalAnnotationDone;
    context::WidgetId boxId{};

    bool revealVocables{false};
    Mode mode{Mode::shuffle};

    std::unique_ptr<multimedia::MpvWrapper> mpv;
};
} // namespace gui
