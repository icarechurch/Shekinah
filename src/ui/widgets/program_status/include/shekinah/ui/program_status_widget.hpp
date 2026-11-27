#pragma once

#include "shekinah/ui/program_status_types.hpp"

#include <QString>
#include <QWidget>

#include <cstddef>
#include <functional>
class QLabel;
class QHBoxLayout;
class QLineEdit;
class QListWidget;
class QProgressBar;
class QPushButton;
class QScreen;
class QStackedWidget;
class QVBoxLayout;

namespace shekinah::ui {

class ProgramStatusWidget final : public QWidget {
public:
    explicit ProgramStatusWidget(QWidget* parent = nullptr);

    void set_program(const domain::ProgramState& program);
    void set_program_title(QString title);
    void set_slide_preview(QString text,
                           QString background_path = {},
                           domain::FitMode background_fit = domain::FitMode::cover);
    void set_live_details(LiveSlideDetails details);
    void set_item_cues(QString item_title, std::vector<QuickCueItem> cues);
    void set_output_status(QString target_name, QString mode_label, bool output_visible);
    void set_memory_status(std::size_t working_set_bytes, bool live_media_admitted);
    [[nodiscard]] bool global_controls_below_status() const noexcept;

    void set_previous_handler(std::function<void()> handler) { previous_handler_ = std::move(handler); }
    void set_next_handler(std::function<void()> handler) { next_handler_ = std::move(handler); }
    void set_clear_handler(std::function<void()> handler) { clear_handler_ = std::move(handler); }
    void set_black_handler(std::function<void()> handler) { black_handler_ = std::move(handler); }
    void set_show_screen_handler(std::function<void(QScreen*)> handler) {
        show_screen_handler_ = std::move(handler);
    }
    void set_show_movable_handler(std::function<void()> handler) {
        show_movable_handler_ = std::move(handler);
    }
    void set_hide_output_handler(std::function<void()> handler) {
        hide_output_handler_ = std::move(handler);
    }
    void set_inspect_live_handler(std::function<void()> handler) {
        inspect_live_handler_ = std::move(handler);
    }
    void set_choose_background_handler(std::function<void()> handler) {
        choose_background_handler_ = std::move(handler);
    }
    void set_take_cue_handler(std::function<void(domain::CueId)> handler) {
        take_cue_handler_ = std::move(handler);
    }

private:
    void build_header(QVBoxLayout* layout);
    void build_monitor(QVBoxLayout* layout);
    void build_monitor_preview(QWidget* monitor_row, QHBoxLayout* monitor_layout);
    void build_monitor_actions(QWidget* monitor_row, QHBoxLayout* monitor_layout);
    void build_screen_selector(QVBoxLayout* layout);
    void build_status_strip(QVBoxLayout* layout);
    void build_global_controls(QVBoxLayout* layout);
    void build_drawer_tabs(QVBoxLayout* layout);
    void build_drawer_pages(QVBoxLayout* layout);
    void build_live_drawer_page();
    void build_next_up_card(QWidget* live_page, QVBoxLayout* live_page_layout);
    void build_screens_drawer_page();
    void build_resource_governor(QWidget* screens_page, QVBoxLayout* screens_layout);
    void build_cues_drawer_page();
    void apply_theme();
    void set_active_tab(int index);
    void update_status_chrome();
    void refresh_screens_list();
    void apply_cue_filter();
    void open_screen_menu();

    QWidget* slide_preview_{nullptr};
    QLabel* mode_label_{nullptr};
    QLabel* cue_label_{nullptr};
    QLabel* output_target_label_{nullptr};
    QLabel* audience_mode_label_{nullptr};
    QWidget* global_controls_{nullptr};
    QPushButton* previous_button_{nullptr};
    QPushButton* next_button_{nullptr};
    QPushButton* clear_button_{nullptr};
    QPushButton* black_button_{nullptr};
    QPushButton* quick_clear_button_{nullptr};
    QPushButton* rail_clear_button_{nullptr};
    QPushButton* rail_black_button_{nullptr};
    QPushButton* rail_audience_button_{nullptr};
    QPushButton* rail_background_button_{nullptr};
    QPushButton* rail_inspector_button_{nullptr};
    QPushButton* screen_menu_button_{nullptr};
    QPushButton* status_screen_indicator_{nullptr};
    QPushButton* status_media_indicator_{nullptr};
    QPushButton* status_audience_indicator_{nullptr};
    QProgressBar* slide_progress_{nullptr};

    QPushButton* tab_live_button_{nullptr};
    QPushButton* tab_screens_button_{nullptr};
    QPushButton* tab_cues_button_{nullptr};
    QStackedWidget* drawer_stack_{nullptr};

    QLabel* live_item_title_label_{nullptr};
    QLabel* live_section_badge_{nullptr};
    QLabel* live_slide_counter_label_{nullptr};
    QLabel* live_lyrics_label_{nullptr};
    QLabel* live_asset_label_{nullptr};
    QPushButton* live_edit_button_{nullptr};
    QPushButton* live_bg_button_{nullptr};
    QPushButton* next_up_header_button_{nullptr};
    QWidget* next_up_card_{nullptr};
    QLabel* next_section_badge_{nullptr};
    QLabel* next_summary_label_{nullptr};
    QPushButton* next_up_action_button_{nullptr};

    QListWidget* screens_list_{nullptr};
    QProgressBar* memory_bar_{nullptr};
    QLabel* memory_status_label_{nullptr};

    QLabel* cues_count_label_{nullptr};
    QListWidget* cues_list_{nullptr};
    QLineEdit* cues_filter_input_{nullptr};

    domain::ProgramState current_program_{};
    LiveSlideDetails live_details_{};
    std::vector<QuickCueItem> item_cues_;
    QString active_item_title_;
    bool output_visible_{false};
    int active_tab_{0};
    std::size_t last_working_set_bytes_{0};
    bool last_live_media_admitted_{true};

    std::function<void()> previous_handler_;
    std::function<void()> next_handler_;
    std::function<void()> clear_handler_;
    std::function<void()> black_handler_;
    std::function<void(QScreen*)> show_screen_handler_;
    std::function<void()> show_movable_handler_;
    std::function<void()> hide_output_handler_;
    std::function<void()> inspect_live_handler_;
    std::function<void()> choose_background_handler_;
    std::function<void(domain::CueId)> take_cue_handler_;
};

}  // namespace shekinah::ui
