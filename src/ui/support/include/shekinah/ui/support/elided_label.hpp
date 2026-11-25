#pragma once

#include <QFontMetrics>
#include <QLabel>
#include <QPainter>
#include <QSizePolicy>
#include <QStyle>
#include <QStyleOption>

namespace shekinah::ui {

/// Single-line label that preserves text() while eliding overflow cleanly during paint.
class ElidedLabel final : public QLabel {
public:
    explicit ElidedLabel(const QString& text,
                         QWidget* parent = nullptr,
                         Qt::TextElideMode mode = Qt::ElideRight)
        : QLabel(text, parent), elide_mode_(mode) {
        setMinimumWidth(0);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }

    explicit ElidedLabel(QWidget* parent = nullptr, Qt::TextElideMode mode = Qt::ElideRight)
        : ElidedLabel(QString{}, parent, mode) {}

    [[nodiscard]] QSize minimumSizeHint() const override {
        return {0, QLabel::minimumSizeHint().height()};
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        if (wordWrap()) {
            QLabel::paintEvent(event);
            return;
        }
        Q_UNUSED(event);
        QPainter painter{this};
        QStyleOption opt;
        opt.initFrom(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
        drawFrame(&painter);

        const QRect cr = contentsRect();
        const QFontMetrics fm{font()};
        const QString elided = fm.elidedText(text(), elide_mode_, std::max(0, cr.width()));
        style()->drawItemText(&painter,
                              cr,
                              static_cast<int>(alignment()),
                              palette(),
                              isEnabled(),
                              elided,
                              foregroundRole());
    }

private:
    Qt::TextElideMode elide_mode_{Qt::ElideRight};
};

}  // namespace shekinah::ui
