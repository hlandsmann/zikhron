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
#include <misc/Language.h>
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
                       std::unique_ptr<TabCard> _tabCardChi,
                       std::unique_ptr<TabCard> _tabCardJpn,
                       std::unique_ptr<TabVideo> _tabVideoChi,
                       std::unique_ptr<TabVideo> _tabVideoJpn)
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
    , tabCardChi{std::move(_tabCardChi)}
    , tabCardJpn{std::move(_tabCardJpn)}
    , tabVideoChi{std::move(_tabVideoChi)}
    , tabVideoJpn{std::move(_tabVideoJpn)}
{
    tabVideoChi->connect_playVideoSet(&MainWindow::slot_playVideoSet, this);
    tabVideoJpn->connect_playVideoSet(&MainWindow::slot_playVideoSet, this);

    tabVideoChi->connect_playVideoSet(&TabCard::slot_playVideoSet, tabCardChi);
    tabVideoJpn->connect_playVideoSet(&TabCard::slot_playVideoSet, tabCardJpn);
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
        switch (language) {
        case Language::chinese:
            tabCardChi->displayOnLayer(layer);
            break;
        case Language::japanese:
            tabCardJpn->displayOnLayer(layer);
            break;
        case Language::languageCount:
            break;
        }
        break;
    case ActiveTab::video:
        switch (language) {
        case Language::chinese:
            tabVideoChi->displayOnLayer(layer);
            break;
        case Language::japanese:
            tabVideoJpn->displayOnLayer(layer);
            break;
        case Language::languageCount:
            break;
        }
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

        tabCardChi->setUp(*mainLayer);
        tabCardJpn->setUp(*mainLayer);
        tabVideoChi->setUp(*mainLayer);
        tabVideoJpn->setUp(*mainLayer);

        tabCardChi->setLanguage(Language::chinese);
        tabVideoChi->setLanguage(Language::chinese);
        tabCardJpn->setLanguage(Language::japanese);
        tabVideoJpn->setLanguage(Language::japanese);
    }
}

void MainWindow::slot_playVideoSet(database::VideoSetPtr /*videoSet*/)
{
    activeTab = ActiveTab::cards;
}
} // namespace gui
