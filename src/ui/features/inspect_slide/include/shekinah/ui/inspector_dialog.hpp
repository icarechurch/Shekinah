#pragma once

#include "shekinah/domain/model.hpp"

#include <QDialog>

namespace shekinah::ui {

class InspectorDialog final : public QDialog {
public:
    explicit InspectorDialog(QWidget* parent = nullptr);

    void show_inspector(domain::ContentRef content, domain::CueId cue);
    [[nodiscard]] const domain::ContentRef& requested_content() const noexcept { return content_; }
    [[nodiscard]] const domain::CueId& requested_cue() const noexcept { return cue_; }

private:
    domain::ContentRef content_{domain::ContentRef::song("selected")};
    domain::CueId cue_;
};

}  // namespace shekinah::ui
