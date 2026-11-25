#pragma once

#include "shekinah/domain/model.hpp"

#include <QColor>
#include <QPoint>
#include <QRect>
#include <QWidget>

#include <functional>
#include <optional>
#include <utility>
#include <vector>

class QScrollArea;

namespace shekinah::ui {

struct CueView {
    domain::CueId id;
    QString section;
    QString summary;
    int number{0};
    QString background_path;
    domain::FitMode background_fit{domain::FitMode::cover};
    QString footer_label;
    bool show_text{true};

    CueView(domain::CueId cue_id,
            QString cue_section,
            QString cue_summary,
            int cue_number = 0,
            QString cue_background_path = {},
            domain::FitMode cue_background_fit = domain::FitMode::cover,
            QString cue_footer_label = {},
            bool cue_show_text = true)
        : id(std::move(cue_id)),
          section(std::move(cue_section)),
          summary(std::move(cue_summary)),
          number(cue_number),
          background_path(std::move(cue_background_path)),
          background_fit(cue_background_fit),
          footer_label(std::move(cue_footer_label)),
          show_text(cue_show_text) {}
};

class CueGridWidget final : public QWidget {
public:
    using TakeLiveHandler = std::function<void(domain::CueId)>;
    using ContextHandler = std::function<void(domain::CueId, QPoint)>;
    using AssetDropHandler = std::function<void(domain::CueId, domain::AssetId)>;

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
    [[nodiscard]] QRect visual_rect(const domain::CueId& cue) const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    class CueCard;
    void apply_theme();
    void rebuild_cards();
    void relayout_cards(bool force);
    void handle_left_click(const domain::CueId& cue);
    void handle_right_click(const domain::CueId& cue, QPoint global_position);
    void handle_asset_drop(const domain::CueId& cue, const domain::AssetId& asset);

    std::vector<CueView> cues_;
    std::vector<CueCard*> cards_;
    std::optional<domain::CueId> live_cue_;
    int current_columns_{0};
    TakeLiveHandler take_live_handler_;
    ContextHandler context_handler_;
    AssetDropHandler asset_drop_handler_;
    QWidget* grid_container_{nullptr};
    QScrollArea* grid_scroll_{nullptr};
    QScrollArea* empty_scroll_{nullptr};
    QWidget* empty_state_{nullptr};
    std::function<void()> new_song_handler_;
    std::function<void()> import_song_handler_;
    std::function<void()> import_sermon_handler_;
    std::function<void()> import_asset_handler_;
};

}  // namespace shekinah::ui
