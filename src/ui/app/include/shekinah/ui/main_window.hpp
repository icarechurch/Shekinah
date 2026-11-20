#pragma once

#include <QMainWindow>

namespace shekinah::ui {

class LibraryDockWidget;
class OperatorPage;

class ShekinahWindow final : public QMainWindow {
public:
    explicit ShekinahWindow(QWidget* parent = nullptr);

    [[nodiscard]] OperatorPage* operator_page() const noexcept { return operator_page_; }
    [[nodiscard]] LibraryDockWidget* library_dock() const noexcept { return library_dock_; }

private:
    OperatorPage* operator_page_{nullptr};
    LibraryDockWidget* library_dock_{nullptr};
};

}  // namespace shekinah::ui
