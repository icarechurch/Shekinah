#pragma once

#include "shekinah/domain/model.hpp"

#include <QDialog>

#include <functional>
#include <string>
#include <utility>
#include <vector>

class QComboBox;
class QLineEdit;
class QPlainTextEdit;

namespace shekinah::ui {

class InspectorDialog final : public QDialog {
public:
    using SaveHandler = std::function<void(domain::CueId,
                                           std::string,
                                           std::vector<std::string>,
                                           domain::FitMode)>;

    explicit InspectorDialog(QWidget* parent = nullptr);

    void show_inspector(domain::ContentRef content, domain::CueId cue);
    void show_slide(domain::ContentRef content,
                    domain::CueId cue,
                    std::string section,
                    std::vector<std::string> lines,
                    domain::FitMode fit);
    void set_save_handler(SaveHandler handler) { save_handler_ = std::move(handler); }
    [[nodiscard]] const domain::ContentRef& requested_content() const noexcept { return content_; }
    [[nodiscard]] const domain::CueId& requested_cue() const noexcept { return cue_; }

private:
    void apply_theme();
    void emit_save();

    domain::ContentRef content_{domain::ContentRef::song("selected")};
    domain::CueId cue_;
    QLineEdit* section_{nullptr};
    QPlainTextEdit* lyrics_{nullptr};
    QComboBox* fit_mode_{nullptr};
    SaveHandler save_handler_;
};

}  // namespace shekinah::ui
