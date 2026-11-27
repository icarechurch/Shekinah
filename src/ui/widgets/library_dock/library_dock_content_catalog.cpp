#include "shekinah/ui/library_dock_widget.hpp"

#include "library_components.hpp"

#include <QLineEdit>
#include <QListWidget>

namespace shekinah::ui {
using namespace library_internal;

void LibraryDockWidget::add_song(domain::Song song) {
    const QString id = QString::fromStdString(song.id.value);
    QListWidgetItem* item = nullptr;
    for (int index = 0; index != songs_->count(); ++index) {
        if (songs_->item(index)->data(kContentIdRole).toString() == id) {
            item = songs_->item(index);
            break;
        }
    }
    if (item == nullptr) {
        item = new QListWidgetItem{};
        item->setData(kContentIdRole, id);
        songs_->addItem(item);
    }
    item->setText(QString::fromStdString(song.title));
    item->setData(kContentKindRole, static_cast<int>(domain::ContentKind::song));
    item->setData(kSlideCountRole, static_cast<int>(song.slides.size()));
    item->setData(kHasBgRole, song.default_background.has_value());
    item->setToolTip(QStringLiteral("%1\n%2 slide%3 · Double-click or drag to Service Order")
                         .arg(item->text())
                         .arg(song.slides.size())
                         .arg(song.slides.size() == 1 ? QString() : QStringLiteral("s")));
    apply_filter_to_content(item, search_->text());
    update_tab_and_header_state();
    update_empty_state();
}

void LibraryDockWidget::add_sermon(domain::SermonDeck deck) {
    const QString id = QString::fromStdString(deck.id.value);
    QListWidgetItem* item = nullptr;
    for (int index = 0; index != sermons_->count(); ++index) {
        if (sermons_->item(index)->data(kContentIdRole).toString() == id) {
            item = sermons_->item(index);
            break;
        }
    }
    if (item == nullptr) {
        item = new QListWidgetItem{};
        item->setData(kContentIdRole, id);
        sermons_->addItem(item);
    }
    item->setText(QString::fromStdString(deck.title));
    item->setData(kContentKindRole, static_cast<int>(domain::ContentKind::sermon));
    item->setData(kSlideCountRole, static_cast<int>(deck.slides.size()));
    item->setData(kHasBgRole, false);
    item->setToolTip(QStringLiteral("%1\n%2 slide%3 · Double-click or drag to Service Order")
                         .arg(item->text())
                         .arg(deck.slides.size())
                         .arg(deck.slides.size() == 1 ? QString() : QStringLiteral("s")));
    apply_filter_to_content(item, search_->text());
    update_tab_and_header_state();
    update_empty_state();
}

}  // namespace shekinah::ui
