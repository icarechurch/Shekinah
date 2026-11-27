#include "shekinah/ui/reflow_text_edit.hpp"

#include "shekinah/ui/support/theme.hpp"

namespace shekinah::ui {

ReflowTextEdit::ReflowTextEdit(QWidget* parent) : QPlainTextEdit(parent) {
    setObjectName(QStringLiteral("reflow-text-edit"));
    setPlaceholderText(QStringLiteral("Enter lyric text. Ctrl+Enter creates a slide split."));
    QObject::connect(this, &QPlainTextEdit::textChanged, this, [this] {
        synchronize_document_text();
    });
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { rebuild_markers(); });
}

}  // namespace shekinah::ui
