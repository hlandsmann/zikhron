#include "MainWindow.h"

#include <DisplayVideo.h>
#include <TabCard.h>
#include <TabVideo.h>
#include <context/Fonts.h>
#include <context/Texture.h>
#include <context/Theme.h>
#include <context/WidgetId.h>
#include <context/imglog.h>
#include <database/VideoSet.h>
#include <imgui.h>
#include <utils/format.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/Separator.h>
#include <widgets/ToggleButtonGroup.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <initializer_list>
#include <memory>
#include <utility>

namespace gui {

MainWindow::MainWindow(std::shared_ptr<context::Theme> _theme,
                       std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator,
                       std::unique_ptr<TabCard> _tabCard,
                       std::unique_ptr<TabVideo> _tabVideo)
    : theme{std::move(_theme)}
    , boxRect{std::make_shared<widget::layout::Rect>()}
    , box{std::make_shared<widget::Box>(widget::WidgetInit{
              .theme = theme,
              .widgetIdGenerator = std::move(widgetIdGenerator),
              .rect = boxRect,
              .horizontalAlign = widget::layout::Align::start,
              .verticalAlign = widget::layout::Align::start,
              .parent = std::weak_ptr<widget::Widget>{}

      })}
    , tabCard{std::move(_tabCard)}
    , tabVideo{std::move(_tabVideo)}
{
    tabVideo->connect_playVideoSet(&MainWindow::slot_playVideoSet, this);
    tabVideo->connect_playVideoSet(&TabCard::slot_playVideoSet, tabCard);
}

void MainWindow::arrange(const widget::layout::Rect& rect)
{
    needArrange |= (*boxRect != rect);
    needArrange |= box->arrangeIsNecessary();

    *boxRect = rect;
    imglog::log("mainWindow arr, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    // if (needArrange) {
    needArrange = box->arrange(rect);
    // }
}

void MainWindow::doImGui()
{
    box->start();

    {
        auto& tabWindow = box->next<widget::Window>();
        auto droppedWindow = tabWindow.dropWindow();
        tabWindow.start();
        auto& tabBox = tabWindow.next<widget::Box>();
        tabBox.start();
        activeTab = static_cast<ActiveTab>(tabBox.next<widget::ToggleButtonGroup>().Active(static_cast<unsigned>(activeTab)));
        tabBox.next<widget::Separator>();
        language = static_cast<Language>(tabBox.next<widget::ToggleButtonGroup>().Active(static_cast<unsigned>(language)));
    }
    auto& layer = box->next<widget::Layer>();
    switch (activeTab) {
    case ActiveTab::cards:
        tabCard->displayOnLayer(layer);
        break;
    case ActiveTab::video:
        tabVideo->displayOnLayer(layer);
        break;
    case ActiveTab::audio:
        [[fallthrough]];
    case ActiveTab::configure:
        break;
    }

    // bool show_demo_window = true;
    // if (show_demo_window) {
    //     // ImGui::SetNextWindowFocus();
    //     ImGui::ShowDemoWindow(&show_demo_window);
    // }
}

void MainWindow::setup()
{
    using Align = widget::layout::Align;
    using namespace widget::layout;
    box->setPadding(0.F);
    {
        auto& toggleButtonMenu = *box->add<widget::Window>(Align::start, width_fixed, height_expand, "toggleButtonMenu");
        auto& tmbBox = *toggleButtonMenu.add<widget::Box>(Align::end, widget::Orientation::vertical);

        // tmbBox->setFlipChildrensOrientation(false);
        tmbBox.setPadding(0);

        tmbBox.add<widget::ToggleButtonGroup>(Align::start, widget::Orientation::vertical,
                                              std::initializer_list<context::Image>{
                                                      context::Image::cards,
                                                      context::Image::video,
                                                      context::Image::audio,
                                                      context::Image::configure_app});
        tmbBox.add<widget::Separator>(Align::end, 0.F, 48.F);

        tmbBox.add<widget::ToggleButtonGroup>(Align::start, widget::Orientation::vertical,
                                              std::initializer_list<context::Image>{
                                                      context::Image::flag_china,
                                                      context::Image::flag_japan,
                                                      // context::Image::flag_brazil,
                                                      // context::Image::flag_israel,
                                                      // context::Image::flag_russia,
                                                      // context::Image::flag_spain,
                                                      // context::Image::flag_uk,
                                                      });
    }
    {
        auto mainLayer = box->add<widget::Layer>(Align::start);
        mainLayer->setExpandType(width_expand, height_expand);
        mainLayer->setName("mainLayer");

        tabCard->setUp(*mainLayer);
        tabVideo->setUp(*mainLayer);
    }
}

void MainWindow::slot_playVideoSet(database::VideoSetPtr /*videoSet*/)
{
    activeTab = ActiveTab::cards;
}
} // namespace gui
