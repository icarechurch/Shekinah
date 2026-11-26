#pragma once

#include "shekinah/ui/cue_grid_widget.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {

class CueGridWidget::CueCard final : public QWidget {
public:
    CueCard(const CueView& cue, std::size_t index, CueGridWidget& owner)
        : QWidget(&owner), cue_(cue), index_(index), owner_(owner) {
        setObjectName(QStringLiteral("cue-%1").arg(QString::fromStdString(cue.id.value)));
        setMinimumSize(120, 100);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setAcceptDrops(true);
        setProperty("cue-section", cue.section);
        setProperty("cue-summary", cue.summary);
        setProperty("cue-number", cue.number);
        setProperty(
            "cue-footer-label",
            cue.footer_label.isEmpty() ? cue.section : cue.footer_label);
        setProperty("cue-section-color", CueGridWidget::footer_color_for_section(cue.section));
        setProperty("cue-background-path", cue.background_path);
        setProperty("cue-show-text", cue.show_text);
        setProperty("cue-live", false);
        setProperty("cue-drop-target", false);
        setToolTip(cue.summary);
        slide_renderer_.set_content(media::SlideRenderData{
            cue.summary,
            cue.background_path,
            cue.background_fit,
            cue.show_text,
            true,
        });
    }

    int heightForWidth(int width) const override {
        const int preview_width = std::max(1, width - 4);
        const int preview_height =
            static_cast<int>(std::lround(static_cast<double>(preview_width) * 9.0 / 16.0));
        return std::max(100, preview_height + 36);
    }

    bool hasHeightForWidth() const override { return true; }

    QSize sizeHint() const override { return {240, heightForWidth(240)}; }

    QSize minimumSizeHint() const override { return {120, heightForWidth(120)}; }

    QRect preview_rect() const {
        constexpr int border = 2;
        constexpr int footer = 32;
        return rect().adjusted(border, border, -border, -footer - border);
    }

    void set_live(bool live) {
        setProperty("cue-live", live);
        update();
    }

protected:
    enum class DropZone { on_card, insert_before, insert_after };

    DropZone drop_zone_for(QPoint pos) const {
        const int edge = width() >= 32 ? std::clamp(width() / 10, 8, 16) : 0;
        if (edge > 0 && pos.x() < edge) {
            return DropZone::insert_before;
        }
        if (edge > 0 && pos.x() >= width() - edge) {
            return DropZone::insert_after;
        }
        return DropZone::on_card;
    }

    void update_drag_hover(const QMimeData* mime, QPoint pos) {
        if (!asset_id_from_mime_data(mime).has_value()) {
            setProperty("cue-drop-target", false);
            update();
            owner_.clear_insert_target();
            return;
        }
        const auto zone = drop_zone_for(pos);
        if (zone == DropZone::on_card) {
            owner_.clear_insert_target();
            setProperty("cue-drop-target", true);
            update();
        } else {
            setProperty("cue-drop-target", false);
            update();
            const std::size_t insert_index =
                zone == DropZone::insert_before ? index_ : index_ + 1;
            owner_.set_insert_target_for_card(
                index_, insert_index, zone == DropZone::insert_before);
        }
    }

    void dragEnterEvent(QDragEnterEvent* event) override {
        if (asset_id_from_mime_data(event->mimeData()).has_value()) {
            update_drag_hover(event->mimeData(), event->position().toPoint());
            event->acceptProposedAction();
        } else {
            setProperty("cue-drop-target", false);
            update();
            owner_.clear_insert_target();
            event->ignore();
        }
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (asset_id_from_mime_data(event->mimeData()).has_value()) {
            update_drag_hover(event->mimeData(), event->position().toPoint());
            event->acceptProposedAction();
        } else {
            setProperty("cue-drop-target", false);
            update();
            owner_.clear_insert_target();
            event->ignore();
        }
    }

    void dragLeaveEvent(QDragLeaveEvent* event) override {
        setProperty("cue-drop-target", false);
        update();
        owner_.clear_insert_target();
        event->accept();
    }

    void dropEvent(QDropEvent* event) override {
        setProperty("cue-drop-target", false);
        update();
        owner_.clear_insert_target();
        const auto asset = asset_id_from_mime_data(event->mimeData());
        if (!asset.has_value()) {
            event->ignore();
            return;
        }
        const auto zone = drop_zone_for(event->position().toPoint());
        if (zone == DropZone::on_card) {
            owner_.handle_asset_drop(cue_.id, *asset);
        } else {
            const std::size_t insert_index =
                zone == DropZone::insert_before ? index_ : index_ + 1;
            owner_.handle_asset_insert(insert_index, *asset);
        }
        event->acceptProposedAction();
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            owner_.handle_left_click(cue_.id);
        } else if (event->button() == Qt::RightButton) {
            owner_.handle_right_click(cue_.id, mapToGlobal(event->position().toPoint()));
        }
        event->accept();
    }

    void paintEvent(QPaintEvent*) override {
        const auto& p = ThemeManager::instance().palette();
        QPainter painter{this};
        painter.fillRect(rect(), p.cue_card_bg);
        const auto preview = preview_rect();
        slide_renderer_.paint(painter, preview);

        const QRect footer{2, height() - 34, width() - 4, 32};
        painter.fillRect(footer, CueGridWidget::footer_color_for_section(cue_.section));
        painter.setPen(Qt::white);
        const int footer_px = std::clamp(width() / 15, 12, 16);
        QFont footer_font = painter.font();
        footer_font.setPixelSize(footer_px);
        footer_font.setBold(true);
        painter.setFont(footer_font);
        const auto number = cue_.number > 0
                                ? QString::number(cue_.number)
                                : QString::fromStdString(cue_.id.value);
        const QFontMetrics num_fm{footer_font};
        const int number_width = num_fm.horizontalAdvance(number);
        const QRect number_rect = footer.adjusted(6, 0, -6, 0);
        painter.drawText(number_rect,
                         Qt::AlignVCenter | Qt::AlignLeft,
                         num_fm.elidedText(number, Qt::ElideRight, number_rect.width()));

        const auto footer_label = cue_.footer_label.isEmpty() ? cue_.section : cue_.footer_label;
        if (!footer_label.isEmpty()) {
            const int label_left = 6 + number_width + 8;
            const QRect label_rect = footer.adjusted(label_left, 0, -6, 0);
            if (label_rect.width() > 6) {
                QFont label_font = footer_font;
                label_font.setBold(false);
                painter.setFont(label_font);
                const QFontMetrics label_fm{label_font};
                painter.drawText(
                    label_rect,
                    Qt::AlignVCenter | Qt::AlignLeft,
                    label_fm.elidedText(footer_label, Qt::ElideRight, label_rect.width()));
            }
        }

        const auto live = property("cue-live").toBool();
        const auto drop_target = property("cue-drop-target").toBool();
        painter.setPen(QPen{drop_target
                                ? p.drop_highlight
                                : live ? p.accent
                                       : p.cue_card_border,
                            static_cast<qreal>(drop_target || live ? 2 : 1)});
        painter.drawRect(rect().adjusted(1, 1, -2, -2));
    }

private:
    CueView cue_;
    std::size_t index_{0};
    CueGridWidget& owner_;
    media::SlideRenderer slide_renderer_;
};



}  // namespace shekinah::ui

