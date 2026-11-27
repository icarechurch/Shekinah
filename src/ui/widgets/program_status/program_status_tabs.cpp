#include "shekinah/ui/program_status_widget.hpp"
#include "program_status_icons.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/cue_grid_widget.hpp"
#include "shekinah/ui/support/elided_label.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QAbstractItemView>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {
void ProgramStatusWidget::set_active_tab(int index) {
    const int clamped = std::clamp(index, 0, drawer_stack_->count() - 1);
    active_tab_ = clamped;
    drawer_stack_->setCurrentIndex(clamped);

    const auto& p = ThemeManager::instance().palette();
    const std::pair<QPushButton*, program_status::IconGlyph> tabs[] = {
        {tab_live_button_, program_status::IconGlyph::layers},
        {tab_screens_button_, program_status::IconGlyph::display},
        {tab_cues_button_, program_status::IconGlyph::list},
    };
    for (int i = 0; i < 3; ++i) {
        const bool active = (i == clamped);
        auto* btn = tabs[i].first;
        btn->setChecked(active);
        btn->setStyleSheet(active
                               ? QStringLiteral("background-color: %1; border: none;")
                                     .arg(p.tab_active_bg.name())
                               : QStringLiteral("background-color: %1; border: none;")
                                     .arg(p.tab_inactive_bg.name()));
        btn->setIcon(program_status::make_glyph_icon(
            tabs[i].second,
            active ? p.tab_active_fg : p.tab_inactive_fg,
            15));
    }
    if (clamped == 1) {
        refresh_screens_list();
    }
}

}  // namespace shekinah::ui
