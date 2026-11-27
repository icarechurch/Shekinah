#pragma once

#include "shekinah/ui/cue_view.hpp"

#include <QColor>
#include <QPoint>
#include <QRect>
#include <QWidget>

#include <cstddef>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

class QScrollArea;
class QVBoxLayout;

namespace shekinah::ui {

class CueGridWidget final : public QWidget {
public:
    using TakeLiveHandler = std::function<void(domain::CueId)>;
    using ContextHandler = std::function<void(domain::CueId, QPoint)>;
    using AssetDropHandler = std::function<void(domain::CueId, domain::AssetId)>;
    using AssetInsertHandler = std::function<void(std::size_t, domain::AssetId)>;

    explicit CueGridWidget(QWidget* parent = nullptr);

    static int column_count_for_geometry(int viewport_width, int screen_width);
    static QColor footer_color_for_section(const QString& section);
    void set_cues(std::vector<CueView> cues);
    void set_authoring_handlers(std::function<void()> new_song,
                                std::function<void()> import_song,
                                std::function<void()> import_sermon,
                                std::function<void()> import_asset);
    void set_live_cue(std::optional<domain::CueId> cue);
    void set_take_live_handler(TakeLiveHandler handler) { take_live_handler_ = std::move(handler); }
    void set_context_handler(ContextHandler handler) { context_handler_ = std::move(handler); }
    void set_asset_drop_handler(AssetDropHandler handler) {
        asset_drop_handler_ = std::move(handler);
    }
    void set_asset_insert_handler(AssetInsertHandler handler) {
        asset_insert_handler_ = std::move(handler);
    }
    [[nodiscard]] QRect visual_rect(const domain::CueId& cue) const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    class CueCard;
    class InsertOverlay;
    struct InsertTarget {
        std::size_t index{0};
        QRect indicator_rect;
    };

    void apply_theme();
    void setup_empty_view(QVBoxLayout* outer_layout);
    void setup_grid_view(QVBoxLayout* outer_layout);
    void rebuild_cards();
    void relayout_cards(bool force);
    void handle_left_click(const domain::CueId& cue);
    void handle_right_click(const domain::CueId& cue, QPoint global_position);
    void handle_asset_drop(const domain::CueId& cue, const domain::AssetId& asset);
    void handle_asset_insert(std::size_t index, const domain::AssetId& asset);
    [[nodiscard]] std::optional<InsertTarget> compute_insert_target(QPoint container_pos) const;
    [[nodiscard]] QRect indicator_rect_for_card_edge(std::size_t card_index, bool left_side) const;
    void set_insert_target(InsertTarget target);
    void set_insert_target_for_card(std::size_t card_index, std::size_t insert_index, bool left_side);
    void clear_insert_target();

    std::vector<CueView> cues_;
    std::vector<CueCard*> cards_;
    std::optional<domain::CueId> live_cue_;
    std::optional<InsertTarget> insert_target_;
    int current_columns_{0};
    TakeLiveHandler take_live_handler_;
    ContextHandler context_handler_;
    AssetDropHandler asset_drop_handler_;
    AssetInsertHandler asset_insert_handler_;
    QWidget* grid_container_{nullptr};
    InsertOverlay* insert_overlay_{nullptr};
    QScrollArea* grid_scroll_{nullptr};
    QScrollArea* empty_scroll_{nullptr};
    QWidget* empty_state_{nullptr};
    std::function<void()> new_song_handler_;
    std::function<void()> import_song_handler_;
    std::function<void()> import_sermon_handler_;
    std::function<void()> import_asset_handler_;
};

}  // namespace shekinah::ui
