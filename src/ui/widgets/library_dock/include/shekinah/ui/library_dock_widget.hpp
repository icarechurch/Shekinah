#pragma once

#include "shekinah/domain/model.hpp"

#include <QDockWidget>

class QListWidget;
class QTabWidget;

namespace shekinah::ui {

enum class LibraryTab { assets, songs, sermons };

class LibraryDockWidget final : public QDockWidget {
public:
    explicit LibraryDockWidget(QWidget* parent = nullptr);

    void show_assets();
    void show_songs();
    void show_sermons();
    void add_asset(domain::AssetRecord asset);
    void add_song(domain::Song song);
    void add_sermon(domain::SermonDeck deck);

    [[nodiscard]] LibraryTab current_tab() const noexcept { return current_tab_; }
    [[nodiscard]] int visible_asset_count() const noexcept;
    [[nodiscard]] int visible_content_count() const noexcept;

private:
    void set_tab(LibraryTab tab, int index);

    LibraryTab current_tab_{LibraryTab::assets};
    QTabWidget* tabs_{nullptr};
    QListWidget* assets_{nullptr};
    QListWidget* songs_{nullptr};
    QListWidget* sermons_{nullptr};
};

}  // namespace shekinah::ui
