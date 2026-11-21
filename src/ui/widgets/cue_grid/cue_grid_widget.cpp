#include "shekinah/ui/cue_grid_widget.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/support/asset_drag.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScreen>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace shekinah::ui {

class CueGridWidget::CueCard final : public QWidget {
public:
    CueCard(const CueView& cue, CueGridWidget& owner) : QWidget(&owner), cue_(cue), owner_(owner) {
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
        return std::max(100, static_cast<int>(std::lround(static_cast<double>(width) * 9.0 / 16.0)) + 32);
    }

    bool hasHeightForWidth() const override { return true; }

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
    void dragEnterEvent(QDragEnterEvent* event) override {
        if (asset_id_from_mime_data(event->mimeData()).has_value()) {
            setProperty("cue-drop-target", true);
            update();
            event->acceptProposedAction();
        } else {
            setProperty("cue-drop-target", false);
            update();
            event->ignore();
        }
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (asset_id_from_mime_data(event->mimeData()).has_value()) {
            setProperty("cue-drop-target", true);
            update();
            event->acceptProposedAction();
        } else {
            setProperty("cue-drop-target", false);
            update();
            event->ignore();
        }
    }

    void dragLeaveEvent(QDragLeaveEvent* event) override {
        setProperty("cue-drop-target", false);
        update();
        event->accept();
    }

    void dropEvent(QDropEvent* event) override {
        setProperty("cue-drop-target", false);
        update();
        const auto asset = asset_id_from_mime_data(event->mimeData());
        if (!asset.has_value()) {
            event->ignore();
            return;
        }
        owner_.handle_asset_drop(cue_.id, *asset);
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
        QPainter painter{this};
        painter.fillRect(rect(), QColor{QStringLiteral("#121212")});
        const auto preview = preview_rect();
        slide_renderer_.paint(painter, preview);

        const QRect footer{2, height() - 34, width() - 4, 32};
        painter.fillRect(footer, CueGridWidget::footer_color_for_section(cue_.section));
        painter.setPen(Qt::white);
        QFont footer_font = painter.font();
        footer_font.setPixelSize(18);
        footer_font.setBold(true);
        painter.setFont(footer_font);
        const auto number = cue_.number > 0
                                ? QString::number(cue_.number)
                                : QString::fromStdString(cue_.id.value);
        painter.drawText(footer.adjusted(4, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft, number);

        const auto footer_label = cue_.footer_label.isEmpty() ? cue_.section : cue_.footer_label;
        if (!footer_label.isEmpty()) {
            QFont label_font = footer_font;
            label_font.setBold(false);
            painter.setFont(label_font);
            painter.drawText(
                footer.adjusted(32, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft, footer_label);
        }

        const auto live = property("cue-live").toBool();
        const auto drop_target = property("cue-drop-target").toBool();
        painter.setPen(QPen{drop_target
                                ? QColor{QStringLiteral("#65d6ff")}
                                : live ? QColor{QStringLiteral("#087cff")}
                                       : QColor{QStringLiteral("#303030")},
                            static_cast<qreal>(drop_target || live ? 2 : 1)});
        painter.drawRect(rect().adjusted(1, 1, -2, -2));
    }

private:
    CueView cue_;
    CueGridWidget& owner_;
    media::SlideRenderer slide_renderer_;
};

CueGridWidget::CueGridWidget(QWidget* parent) : QWidget(parent), grid_container_(new QWidget(this)) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    empty_state_ = new QWidget{this};
    empty_state_->setObjectName(QStringLiteral("presentation-empty-state"));
    auto* empty_layout = new QVBoxLayout{empty_state_};
    empty_layout->setContentsMargins(32, 32, 32, 32);
    auto* title = new QLabel{QStringLiteral("Presentation"), empty_state_};
    title->setObjectName(QStringLiteral("presentation-title"));
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 600;"));
    auto* message = new QLabel{
        QStringLiteral("Add a Song or Sermon to the Service Order, or create content to begin."),
        empty_state_};
    message->setAlignment(Qt::AlignCenter);
    message->setWordWrap(true);
    empty_layout->addStretch(1);
    empty_layout->addWidget(title);
    empty_layout->addWidget(message);
    const auto add_action = [&](const QString& label, const QString& name, auto handler) {
        auto* button = new QPushButton{label, empty_state_};
        button->setObjectName(name);
        button->setMinimumHeight(38);
        QObject::connect(button, &QPushButton::clicked, this, handler);
        empty_layout->addWidget(button);
    };
    add_action(QStringLiteral("New Song"), QStringLiteral("presentation-new-song"), [this] {
        if (new_song_handler_) {
            new_song_handler_();
        }
    });
    add_action(QStringLiteral("Import Song Lyrics"), QStringLiteral("presentation-import-song"), [this] {
        if (import_song_handler_) {
            import_song_handler_();
        }
    });
    add_action(QStringLiteral("Import Sermon Deck"), QStringLiteral("presentation-import-sermon"), [this] {
        if (import_sermon_handler_) {
            import_sermon_handler_();
        }
    });
    add_action(QStringLiteral("Import Asset"), QStringLiteral("presentation-import-asset"), [this] {
        if (import_asset_handler_) {
            import_asset_handler_();
        }
    });
    empty_layout->addStretch(1);
    outer->addWidget(empty_state_);
    grid_scroll_ = new QScrollArea{this};
    grid_scroll_->setObjectName(QStringLiteral("cue-grid-scroll"));
    grid_scroll_->setFrameShape(QFrame::NoFrame);
    grid_scroll_->setWidgetResizable(true);
    grid_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    grid_scroll_->setWidget(grid_container_);
    outer->addWidget(grid_scroll_);
    grid_scroll_->setVisible(false);
    setMinimumWidth(240);
}

QColor CueGridWidget::footer_color_for_section(const QString& section) {
    const auto normalized = section.trimmed().toLower();
    if (normalized.startsWith(QStringLiteral("chorus"))) {
        return QColor{QStringLiteral("#cc1f58")};
    }
    if (normalized.startsWith(QStringLiteral("bridge"))) {
        return QColor{QStringLiteral("#8336c7")};
    }
    if (normalized.startsWith(QStringLiteral("tag"))) {
        return QColor{QStringLiteral("#cf3a32")};
    }
    return QColor{QStringLiteral("#0b80ba")};
}

int CueGridWidget::column_count_for_geometry(int viewport_width, int screen_width) {
    if (viewport_width < 320) {
        return 1;
    }
    if (viewport_width < 520) {
        return 2;
    }
    if (screen_width < 1600) {
        return 3;
    }
    if (screen_width < 2300) {
        return 4;
    }
    if (screen_width < 3200) {
        return 5;
    }
    return 6;
}

void CueGridWidget::set_cues(std::vector<CueView> cues) {
    cues_ = std::move(cues);
    const bool empty = cues_.empty();
    empty_state_->setVisible(empty);
    grid_scroll_->setVisible(!empty);
    rebuild_layout();
}

void CueGridWidget::set_authoring_handlers(std::function<void()> new_song,
                                           std::function<void()> import_song,
                                           std::function<void()> import_sermon,
                                           std::function<void()> import_asset) {
    new_song_handler_ = std::move(new_song);
    import_song_handler_ = std::move(import_song);
    import_sermon_handler_ = std::move(import_sermon);
    import_asset_handler_ = std::move(import_asset);
}

void CueGridWidget::set_live_cue(std::optional<domain::CueId> cue) {
    live_cue_ = std::move(cue);
    for (auto* card : cards_) {
        card->set_live(live_cue_.has_value() &&
                      card->property("cue-id").toString() ==
                          QString::fromStdString(live_cue_->value));
    }
}

QRect CueGridWidget::visual_rect(const domain::CueId& cue) const {
    const auto card = std::find_if(cards_.begin(), cards_.end(), [&](const auto* candidate) {
        return candidate->objectName() == QStringLiteral("cue-%1").arg(QString::fromStdString(cue.value));
    });
    if (card == cards_.end()) {
        return {};
    }
    return QRect(mapFrom(*card, QPoint{0, 0}), (*card)->size());
}

void CueGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rebuild_layout();
}

void CueGridWidget::rebuild_layout() {
    if (grid_container_ == nullptr) {
        return;
    }
    if (auto* old_layout = grid_container_->layout(); old_layout != nullptr) {
        while (auto* item = old_layout->takeAt(0)) {
            if (auto* widget = item->widget(); widget != nullptr) {
                widget->deleteLater();
            }
            delete item;
        }
        delete old_layout;
    }
    cards_.clear();

    auto* layout = new QGridLayout(grid_container_);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    const int screen_width = screen() == nullptr ? 1920 : screen()->size().width();
    const int columns = column_count_for_geometry(width(), screen_width);
    for (std::size_t index = 0; index != cues_.size(); ++index) {
        auto* card = new CueCard{cues_[index], *this};
        card->setProperty("cue-id", QString::fromStdString(cues_[index].id.value));
        card->set_live(live_cue_.has_value() && cues_[index].id == *live_cue_);
        cards_.push_back(card);
        const int row = static_cast<int>(index / static_cast<std::size_t>(columns));
        const int column = static_cast<int>(index % static_cast<std::size_t>(columns));
        layout->addWidget(card, row, column);
    }
}

void CueGridWidget::handle_left_click(const domain::CueId& cue) {
    if (take_live_handler_) {
        take_live_handler_(cue);
    }
}

void CueGridWidget::handle_right_click(const domain::CueId& cue, QPoint global_position) {
    if (context_handler_) {
        context_handler_(cue, global_position);
    }
}

void CueGridWidget::handle_asset_drop(const domain::CueId& cue, const domain::AssetId& asset) {
    if (asset_drop_handler_) {
        asset_drop_handler_(cue, asset);
    }
}

}  // namespace shekinah::ui
