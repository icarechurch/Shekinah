#pragma once

#include <QDialog>

#include <filesystem>
#include <vector>

class QLineEdit;
class QListWidget;

namespace shekinah::ui {

class SermonEditorDialog final : public QDialog {
public:
    explicit SermonEditorDialog(QWidget* parent = nullptr);

    void set_title(QString title);
    void set_files(std::vector<std::filesystem::path> files);
    [[nodiscard]] QString title() const;
    [[nodiscard]] std::vector<std::filesystem::path> files() const;
    [[nodiscard]] int file_count() const noexcept;

private:
    void add_files();
    void move_current(int direction);
    void remove_current();
    void accept_if_valid();

    QLineEdit* title_{nullptr};
    QListWidget* files_{nullptr};
};

}  // namespace shekinah::ui
