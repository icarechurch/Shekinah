#include "shekinah/ui/sermon_editor_dialog.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <utility>

namespace shekinah::ui {

SermonEditorDialog::SermonEditorDialog(QWidget* parent) : QDialog(parent) {
    setObjectName(QStringLiteral("sermon-editor-dialog"));
    setWindowTitle(QStringLiteral("Sermon Deck"));
    auto* layout = new QVBoxLayout{this};
    auto* title_label = new QLabel{QStringLiteral("Title"), this};
    layout->addWidget(title_label);
    title_ = new QLineEdit{this};
    title_->setObjectName(QStringLiteral("sermon-title"));
    layout->addWidget(title_);

    auto* description = new QLabel{
        QStringLiteral("Add media files, then arrange them in the order they should appear."), this};
    description->setObjectName(QStringLiteral("sermon-description"));
    description->setWordWrap(true);
    layout->addWidget(description);

    files_ = new QListWidget{this};
    files_->setObjectName(QStringLiteral("sermon-files"));
    files_->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(files_, 1);

    auto* controls = new QHBoxLayout;
    const auto add_button = [&](const QString& label, const QString& name, auto handler) {
        auto* button = new QPushButton{label, this};
        button->setObjectName(name);
        QObject::connect(button, &QPushButton::clicked, this, handler);
        controls->addWidget(button);
    };
    add_button(QStringLiteral("Add Media"), QStringLiteral("sermon-add-files"), [this] { add_files(); });
    add_button(QStringLiteral("Move Up"), QStringLiteral("sermon-move-up"), [this] { move_current(-1); });
    add_button(QStringLiteral("Move Down"), QStringLiteral("sermon-move-down"), [this] { move_current(1); });
    add_button(QStringLiteral("Remove"), QStringLiteral("sermon-remove"), [this] { remove_current(); });
    layout->addLayout(controls);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Save | QDialogButtonBox::Cancel, this};
    buttons->setObjectName(QStringLiteral("sermon-buttons"));
    QObject::connect(buttons->button(QDialogButtonBox::Save), &QPushButton::clicked, this,
                     [this] { accept_if_valid(); });
    QObject::connect(buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this,
                     &QDialog::reject);
    layout->addWidget(buttons);
    resize(680, 520);

    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

void SermonEditorDialog::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setPalette(ThemeManager::build_qpalette(p));
    setStyleSheet(ThemeManager::dialog_stylesheet(p));
}

void SermonEditorDialog::set_title(QString title) { title_->setText(std::move(title)); }

QString SermonEditorDialog::title() const { return title_->text().trimmed(); }

int SermonEditorDialog::file_count() const noexcept { return files_->count(); }

void SermonEditorDialog::accept_if_valid() {
    if (!title().isEmpty() && files_->count() != 0) {
        accept();
    }
}

}  // namespace shekinah::ui
