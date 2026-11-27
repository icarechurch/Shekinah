#include "shekinah/ui/reflow_text_edit.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QTextCursor>
#include <QTextFormat>

#include <algorithm>

namespace shekinah::ui {

void ReflowTextEdit::rebuild_markers() {
    const auto& p = ThemeManager::instance().palette();
    break_markers_.clear();
    for (const auto& slide_break : document_.breaks()) {
        QTextCursor cursor = textCursor();
        const auto position = qt_position_for_utf8_offset(slide_break.utf8_offset);
        cursor.setPosition(position);
        cursor.setPosition(std::min(position + 1, static_cast<int>(toPlainText().size())),
                           QTextCursor::KeepAnchor);
        QTextEdit::ExtraSelection marker;
        marker.cursor = cursor;
        marker.format.setBackground(p.reflow_break_bg);
        marker.format.setForeground(p.reflow_break_fg);
        marker.format.setProperty(QTextFormat::FullWidthSelection, true);
        break_markers_.push_back(marker);
    }
    setExtraSelections(break_markers_);
}

}  // namespace shekinah::ui
