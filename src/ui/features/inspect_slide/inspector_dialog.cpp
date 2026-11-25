#include "shekinah/ui/inspector_dialog.hpp"

#include "shekinah/ui/support/theme.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace shekinah::ui {

InspectorDialog::InspectorDialog(QWidget* parent) : QDialog(parent) {
    setObjectName(QStringLiteral("inspector-dialog"));
    setWindowFlag(Qt::Tool, true);
    setModal(false);
    setWindowTitle(QStringLiteral("Inspector"));
    auto* layout = new QVBoxLayout(this);
    auto* description = new QLabel{
        QStringLiteral("Edit this cue without changing the live Program."), this};
    description->setObjectName(QStringLiteral("inspector-description"));
    description->setWordWrap(true);
    layout->addWidget(description);

    section_ = new QLineEdit{this};
    section_->setObjectName(QStringLiteral("inspector-section"));
    section_->setPlaceholderText(QStringLiteral("Section label"));
    layout->addWidget(section_);

    lyrics_ = new QPlainTextEdit{this};
    lyrics_->setObjectName(QStringLiteral("inspector-lyrics"));
    lyrics_->setPlaceholderText(QStringLiteral("Slide lyrics"));
    layout->addWidget(lyrics_, 1);

    fit_mode_ = new QComboBox{this};
    fit_mode_->setObjectName(QStringLiteral("inspector-fit-mode"));
    fit_mode_->addItem(QStringLiteral("Contain"));
    fit_mode_->addItem(QStringLiteral("Cover"));
    fit_mode_->addItem(QStringLiteral("Stretch"));
    layout->addWidget(fit_mode_);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Save | QDialogButtonBox::Close, this};
    buttons->setObjectName(QStringLiteral("inspector-buttons"));
    QObject::connect(buttons->button(QDialogButtonBox::Save), &QPushButton::clicked, this,
                     [this] { emit_save(); });
    QObject::connect(buttons->button(QDialogButtonBox::Close), &QPushButton::clicked, this,
                     &QDialog::hide);
    layout->addWidget(buttons);
    resize(420, 360);

    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

void InspectorDialog::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setPalette(ThemeManager::build_qpalette(p));
    setStyleSheet(ThemeManager::dialog_stylesheet(p));
}

void InspectorDialog::show_inspector(domain::ContentRef content, domain::CueId cue) {
    content_ = std::move(content);
    cue_ = std::move(cue);
    section_->clear();
    lyrics_->clear();
    fit_mode_->setCurrentIndex(1);
    setWindowTitle(QStringLiteral("Inspector — %1").arg(QString::fromStdString(cue_.value)));
    show();
    raise();
    activateWindow();
}

void InspectorDialog::show_slide(domain::ContentRef content,
                                 domain::CueId cue,
                                 std::string section,
                                 std::vector<std::string> lines,
                                 domain::FitMode fit) {
    content_ = std::move(content);
    cue_ = std::move(cue);
    section_->setText(QString::fromStdString(section));
    QString lyrics;
    for (std::size_t index = 0; index != lines.size(); ++index) {
        if (index != 0) {
            lyrics += QLatin1Char('\n');
        }
        lyrics += QString::fromStdString(lines[index]);
    }
    lyrics_->setPlainText(lyrics);
    fit_mode_->setCurrentIndex(fit == domain::FitMode::contain ? 0 : fit == domain::FitMode::cover ? 1 : 2);
    setWindowTitle(QStringLiteral("Inspector — %1").arg(QString::fromStdString(cue_.value)));
    show();
    raise();
    activateWindow();
}

void InspectorDialog::emit_save() {
    if (!save_handler_ || cue_.empty()) {
        return;
    }
    std::vector<std::string> lines;
    const auto values = lyrics_->toPlainText().split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    lines.reserve(static_cast<std::size_t>(values.size()));
    for (const auto& value : values) {
        lines.push_back(value.toStdString());
    }
    const auto fit = fit_mode_->currentIndex() == 0
                         ? domain::FitMode::contain
                         : fit_mode_->currentIndex() == 1 ? domain::FitMode::cover : domain::FitMode::stretch;
    save_handler_(cue_, section_->text().trimmed().toStdString(), std::move(lines), fit);
}

}  // namespace shekinah::ui
