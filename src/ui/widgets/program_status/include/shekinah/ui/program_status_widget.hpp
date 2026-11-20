#pragma once

#include "shekinah/domain/model.hpp"

#include <QWidget>

#include <functional>

class QLabel;
class QPushButton;

namespace shekinah::ui {

class ProgramStatusWidget final : public QWidget {
public:
    explicit ProgramStatusWidget(QWidget* parent = nullptr);

    void set_program(const domain::ProgramState& program);
    void set_program_title(QString title);
    [[nodiscard]] bool global_controls_below_status() const noexcept;

    void set_previous_handler(std::function<void()> handler) { previous_handler_ = std::move(handler); }
    void set_next_handler(std::function<void()> handler) { next_handler_ = std::move(handler); }
    void set_clear_handler(std::function<void()> handler) { clear_handler_ = std::move(handler); }
    void set_black_handler(std::function<void()> handler) { black_handler_ = std::move(handler); }

private:
    QLabel* program_title_{nullptr};
    QLabel* mode_label_{nullptr};
    QLabel* cue_label_{nullptr};
    QWidget* global_controls_{nullptr};
    std::function<void()> previous_handler_;
    std::function<void()> next_handler_;
    std::function<void()> clear_handler_;
    std::function<void()> black_handler_;
};

}  // namespace shekinah::ui
