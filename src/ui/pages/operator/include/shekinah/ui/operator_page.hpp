#pragma once

#include "shekinah/domain/model.hpp"

#include <QWidget>

class QListWidget;
class QMenu;

namespace shekinah::ui {

class CueGridWidget;
class InspectorDialog;
class ProgramStatusWidget;

class OperatorPage final : public QWidget {
public:
    explicit OperatorPage(QWidget* parent = nullptr);

    [[nodiscard]] QListWidget* service_order_widget() const noexcept { return service_order_; }
    [[nodiscard]] CueGridWidget* cue_grid() const noexcept { return cue_grid_; }
    [[nodiscard]] ProgramStatusWidget* program_status() const noexcept { return program_status_; }
    [[nodiscard]] InspectorDialog* inspector() const noexcept { return inspector_; }
    QMenu* create_context_menu(const domain::CueId& cue);

private:
    QListWidget* service_order_{nullptr};
    CueGridWidget* cue_grid_{nullptr};
    ProgramStatusWidget* program_status_{nullptr};
    InspectorDialog* inspector_{nullptr};
};

}  // namespace shekinah::ui
