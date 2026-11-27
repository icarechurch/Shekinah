#include "shekinah/ui/reflow_editor_dialog.hpp"

#include "shekinah/ui/reflow_text_edit.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QListWidget>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

namespace shekinah::ui {

ReflowEditorDialog::ReflowEditorDialog(QWidget* parent) : QDialog(parent) {
    setObjectName(QStringLiteral("reflow-editor-dialog"));
    setWindowTitle(QStringLiteral("Song Reflow"));
    auto* layout = new QVBoxLayout(this);
    title_ = new QLineEdit{this};
    title_->setObjectName(QStringLiteral("song-title"));
    title_->setPlaceholderText(QStringLiteral("Song title"));
    layout->addWidget(title_);
    auto* splitter = new QSplitter{Qt::Horizontal, this};
    editor_ = new ReflowTextEdit{splitter};
    thumbnails_ = new QListWidget{splitter};
    thumbnails_->setObjectName(QStringLiteral("reflow-thumbnails"));
    splitter->addWidget(editor_);
    splitter->addWidget(thumbnails_);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    layout->addWidget(splitter);
    auto* actions = new QWidget{this};
    actions->setObjectName(QStringLiteral("song-editor-actions"));
    actions->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto* actions_layout = new QHBoxLayout{actions};
    actions_layout->setContentsMargins(0, 0, 0, 0);
    auto* split_button = new QPushButton{QStringLiteral("Split Slide / Reflow"), actions};
    split_button->setObjectName(QStringLiteral("split-slide-reflow"));
    split_button->setToolTip(QStringLiteral("Insert a slide break at the cursor (Ctrl+Enter)"));
    QObject::connect(split_button, &QPushButton::clicked, this, [this] {
        editor_->insert_slide_break();
    });
    actions_layout->addWidget(split_button);
    actions_layout->addStretch(1);

    auto* buttons = new QDialogButtonBox{
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, actions};
    buttons->setObjectName(QStringLiteral("song-editor-buttons"));
    QObject::connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        if (save_song()) {
            accept();
        }
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    actions_layout->addWidget(buttons);
    layout->addWidget(actions);
    editor_->set_document_changed_handler([this] { refresh_thumbnails(); });
    resize(920, 600);

    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

}  // namespace shekinah::ui
