#pragma once

#include "shekinah/domain/model.hpp"

#include <QDockWidget>

#include <functional>

class QListWidget;
class QTabWidget;
class QLineEdit;

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
    void clear_contents();
    void set_search_text(const QString& text);
    void set_new_song_handler(std::function<void()> handler) { new_song_handler_ = std::move(handler); }
    void set_import_song_handler(std::function<void()> handler) { import_song_handler_ = std::move(handler); }
    void set_new_sermon_handler(std::function<void()> handler) { new_sermon_handler_ = std::move(handler); }
    void set_import_sermon_handler(std::function<void()> handler) { import_sermon_handler_ = std::move(handler); }
    void set_import_asset_handler(std::function<void()> handler) { import_asset_handler_ = std::move(handler); }
    void set_add_content_handler(std::function<void(domain::ContentRef)> handler) {
        add_content_handler_ = std::move(handler);
    }
    void set_edit_song_handler(std::function<void(domain::SongId)> handler) {
        edit_song_handler_ = std::move(handler);
    }
    void activate_song_row(int row);
    void edit_song_row(int row);
    void activate_sermon_row(int row);

    [[nodiscard]] LibraryTab current_tab() const noexcept { return current_tab_; }
    [[nodiscard]] int visible_asset_count() const noexcept;
    [[nodiscard]] int visible_content_count() const noexcept;

private:
    void set_tab(LibraryTab tab, int index);
    void apply_filter();

    LibraryTab current_tab_{LibraryTab::assets};
    QTabWidget* tabs_{nullptr};
    QLineEdit* search_{nullptr};
    QListWidget* assets_{nullptr};
    QListWidget* songs_{nullptr};
    QListWidget* sermons_{nullptr};
    std::function<void()> new_song_handler_;
    std::function<void()> import_song_handler_;
    std::function<void()> new_sermon_handler_;
    std::function<void()> import_sermon_handler_;
    std::function<void()> import_asset_handler_;
    std::function<void(domain::ContentRef)> add_content_handler_;
    std::function<void(domain::SongId)> edit_song_handler_;
};

}  // namespace shekinah::ui
