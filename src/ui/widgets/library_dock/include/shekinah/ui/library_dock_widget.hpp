#pragma once

#include "shekinah/domain/model.hpp"

#include <QCache>
#include <QColor>
#include <QDockWidget>
#include <QIcon>

#include <filesystem>
#include <functional>
#include <optional>
#include <utility>

class QAction;
class QButtonGroup;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QStackedWidget;
class QWidget;

namespace shekinah::ui {

class FlowLayout;
namespace library_internal {
enum class LibraryGlyph;
}

enum class LibraryTab { assets, songs, sermons };

class LibraryDockWidget final : public QDockWidget {
public:
    explicit LibraryDockWidget(QWidget* parent = nullptr);

    void show_assets();
    void show_songs();
    void show_sermons();
    void set_asset_root(std::filesystem::path root);
    void add_asset(domain::AssetRecord asset);
    void add_song(domain::Song song);
    void add_sermon(domain::SermonDeck deck);
    void clear_contents();
    void purge_non_live_caches();
    void set_search_text(const QString& text);
    void set_asset_kind_filter(std::optional<domain::AssetKind> kind);
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
    void set_edit_sermon_handler(std::function<void(domain::SermonId)> handler) {
        edit_sermon_handler_ = std::move(handler);
    }
    void activate_song_row(int row);
    void edit_song_row(int row);
    void activate_sermon_row(int row);
    void edit_sermon_row(int row);

    [[nodiscard]] LibraryTab current_tab() const noexcept { return current_tab_; }
    [[nodiscard]] std::optional<domain::AssetKind> asset_kind_filter() const noexcept {
        return asset_kind_filter_;
    }
    [[nodiscard]] int visible_asset_count() const noexcept;
    [[nodiscard]] int visible_content_count() const noexcept;
    [[nodiscard]] int cached_thumbnail_count() const noexcept { return thumbnail_cache_.count(); }

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setup_title_bar();
    void setup_toolbar(QWidget* container);
    void setup_toolbar_navigation(QWidget* toolbar, FlowLayout* toolbar_layout);
    void setup_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout);
    void setup_asset_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout);
    void setup_song_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout);
    void setup_sermon_toolbar_actions(QWidget* toolbar, FlowLayout* toolbar_layout);
    [[nodiscard]] std::pair<QWidget*, QHBoxLayout*> create_toolbar_action_group(
        QWidget* toolbar, FlowLayout* toolbar_layout);
    [[nodiscard]] QPushButton* add_toolbar_action(QWidget* group_parent,
                                                   QHBoxLayout* group_layout,
                                                   const QString& label,
                                                   const QString& object_name,
                                                   const QString& tooltip,
                                                   library_internal::LibraryGlyph glyph,
                                                   const QColor& icon_color,
                                                   std::function<void()> handler);
    void setup_content(QWidget* container);
    void setup_asset_list();
    void setup_content_lists();
    void setup_content_empty_overlay();
    void setup_interactions();
    void setup_song_interactions();
    void setup_sermon_interactions();
    void setup_search_interactions();
    void apply_theme();
    void set_tab(LibraryTab tab, int index);
    void apply_filter();
    void apply_filter_to_asset(QListWidgetItem* item, const QString& query) const;
    void apply_filter_to_content(QListWidgetItem* item, const QString& query) const;
    void update_tab_and_header_state();
    void update_action_states();
    void update_empty_state();
    void layout_empty_overlay();
    void update_responsive_grid_sizes();
    [[nodiscard]] QIcon load_bounded_thumbnail(const domain::AssetRecord& asset);

    LibraryTab current_tab_{LibraryTab::assets};
    std::optional<domain::AssetKind> asset_kind_filter_;

    QLabel* header_count_badge_{nullptr};
    QPushButton* float_toggle_button_{nullptr};

    QButtonGroup* tab_group_{nullptr};
    QPushButton* assets_tab_button_{nullptr};
    QPushButton* songs_tab_button_{nullptr};
    QPushButton* sermons_tab_button_{nullptr};

    QLineEdit* search_{nullptr};
    QAction* search_action_{nullptr};
    QWidget* asset_filter_bar_{nullptr};
    QButtonGroup* asset_filter_group_{nullptr};
    QPushButton* filter_all_button_{nullptr};
    QPushButton* filter_images_button_{nullptr};
    QPushButton* filter_videos_button_{nullptr};

    QPushButton* import_asset_button_{nullptr};
    QPushButton* new_song_button_{nullptr};
    QPushButton* import_song_button_{nullptr};
    QPushButton* edit_song_button_{nullptr};
    QPushButton* new_sermon_button_{nullptr};
    QPushButton* import_sermon_button_{nullptr};
    QPushButton* edit_sermon_button_{nullptr};

    QStackedWidget* stack_{nullptr};
    QListWidget* assets_{nullptr};
    QListWidget* songs_{nullptr};
    QListWidget* sermons_{nullptr};

    QWidget* empty_overlay_{nullptr};
    QLabel* empty_icon_label_{nullptr};
    QLabel* empty_title_label_{nullptr};
    QLabel* empty_hint_label_{nullptr};

    QIcon shared_video_icon_;
    QCache<QString, QIcon> thumbnail_cache_;
    std::filesystem::path asset_root_;
    std::function<void()> new_song_handler_;
    std::function<void()> import_song_handler_;
    std::function<void()> new_sermon_handler_;
    std::function<void()> import_sermon_handler_;
    std::function<void()> import_asset_handler_;
    std::function<void(domain::ContentRef)> add_content_handler_;
    std::function<void(domain::SongId)> edit_song_handler_;
    std::function<void(domain::SermonId)> edit_sermon_handler_;
};

}  // namespace shekinah::ui
