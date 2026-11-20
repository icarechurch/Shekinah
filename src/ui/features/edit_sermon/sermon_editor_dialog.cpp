#include "shekinah/ui/sermon_editor_dialog.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
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
        QStringLiteral("Add PNG slides, then arrange them in the order they should appear."), this};
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
    add_button(QStringLiteral("Add PNGs"), QStringLiteral("sermon-add-files"), [this] { add_files(); });
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
}

void SermonEditorDialog::set_title(QString title) { title_->setText(std::move(title)); }

void SermonEditorDialog::set_files(std::vector<std::filesystem::path> files) {
    files_->clear();
    for (auto& path : files) {
        auto* item = new QListWidgetItem{QString::fromStdString(path.filename().string()), files_};
        item->setData(Qt::UserRole, QString::fromStdString(path.string()));
    }
}

QString SermonEditorDialog::title() const { return title_->text().trimmed(); }

std::vector<std::filesystem::path> SermonEditorDialog::files() const {
    std::vector<std::filesystem::path> values;
    values.reserve(static_cast<std::size_t>(files_->count()));
    for (int index = 0; index != files_->count(); ++index) {
        values.emplace_back(files_->item(index)->data(Qt::UserRole).toString().toStdString());
    }
    return values;
}

int SermonEditorDialog::file_count() const noexcept { return files_->count(); }

void SermonEditorDialog::add_files() {
    const auto paths = QFileDialog::getOpenFileNames(
        this, QStringLiteral("Add Sermon PNGs"), {}, QStringLiteral("PNG slides (*.png)"));
    for (const auto& path : paths) {
        bool duplicate = false;
        for (int index = 0; index != files_->count(); ++index) {
            duplicate = files_->item(index)->data(Qt::UserRole).toString() == path;
            if (duplicate) {
                break;
            }
        }
        if (!duplicate) {
            auto* item = new QListWidgetItem{QFileInfo{path}.fileName(), files_};
            item->setData(Qt::UserRole, path);
        }
    }
}

void SermonEditorDialog::move_current(int direction) {
    const int current = files_->currentRow();
    const int target = current + direction;
    if (current < 0 || target < 0 || target >= files_->count()) {
        return;
    }
    auto* item = files_->takeItem(current);
    files_->insertItem(target, item);
    files_->setCurrentRow(target);
}

void SermonEditorDialog::remove_current() {
    const int current = files_->currentRow();
    if (current >= 0) {
        delete files_->takeItem(current);
    }
}

void SermonEditorDialog::accept_if_valid() {
    if (!title().isEmpty() && files_->count() != 0) {
        accept();
    }
}

}  // namespace shekinah::ui
