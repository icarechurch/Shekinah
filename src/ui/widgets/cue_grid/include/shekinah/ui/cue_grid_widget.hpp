#pragma once

#include "shekinah/domain/model.hpp"

#include <QPoint>
#include <QRect>
#include <QWidget>

#include <functional>
#include <vector>

namespace shekinah::ui {

struct CueView {
    domain::CueId id;
    QString section;
    QString summary;
};

class CueGridWidget final : public QWidget {
public:
    using TakeLiveHandler = std::function<void(domain::CueId)>;
    using ContextHandler = std::function<void(domain::CueId, QPoint)>;

    explicit CueGridWidget(QWidget* parent = nullptr);

    static int column_count_for_geometry(int viewport_width, int screen_width);
    void set_cues(std::vector<CueView> cues);
    void set_authoring_handlers(std::function<void()> new_song,
                                std::function<void()> import_song,
                                std::function<void()> import_sermon,
                                std::function<void()> import_asset);
    void set_take_live_handler(TakeLiveHandler handler) { take_live_handler_ = std::move(handler); }
    void set_context_handler(ContextHandler handler) { context_handler_ = std::move(handler); }
    [[nodiscard]] QRect visual_rect(const domain::CueId& cue) const;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    class CueButton;
    void rebuild_layout();
    void handle_left_click(const domain::CueId& cue);
    void handle_right_click(const domain::CueId& cue, QPoint global_position);

    std::vector<CueView> cues_;
    std::vector<CueButton*> buttons_;
    TakeLiveHandler take_live_handler_;
    ContextHandler context_handler_;
    QWidget* grid_container_{nullptr};
    QWidget* empty_state_{nullptr};
    std::function<void()> new_song_handler_;
    std::function<void()> import_song_handler_;
    std::function<void()> import_sermon_handler_;
    std::function<void()> import_asset_handler_;
};

}  // namespace shekinah::ui
