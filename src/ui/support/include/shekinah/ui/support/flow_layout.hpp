#pragma once

#include <QLayout>

#include <vector>

namespace shekinah::ui {

/// Responsive wrapping horizontal layout for toolbars that adapt to narrow windows.
class FlowLayout final : public QLayout {
public:
    explicit FlowLayout(QWidget* parent = nullptr,
                        int margin = -1,
                        int h_spacing = -1,
                        int v_spacing = -1);
    ~FlowLayout() override;

    void addItem(QLayoutItem* item) override;

    [[nodiscard]] int horizontalSpacing() const;
    [[nodiscard]] int verticalSpacing() const;
    [[nodiscard]] Qt::Orientations expandingDirections() const override;
    [[nodiscard]] bool hasHeightForWidth() const override;
    [[nodiscard]] int heightForWidth(int width) const override;
    [[nodiscard]] int count() const override;
    [[nodiscard]] QLayoutItem* itemAt(int index) const override;
    [[nodiscard]] QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QLayoutItem* takeAt(int index) override;

private:
    static bool is_item_hidden(const QLayoutItem* item);
    [[nodiscard]] int smart_spacing(int metric) const;
    int do_layout(const QRect& rect, bool test_only) const;

    std::vector<QLayoutItem*> items_;
    int h_space_{-1};
    int v_space_{-1};
};

}  // namespace shekinah::ui
