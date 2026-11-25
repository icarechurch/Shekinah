#include "shekinah/ui/cue_grid_widget.hpp"

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/support/asset_drag.hpp"
#include "shekinah/ui/support/theme.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFontMetrics>
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
    CueGridWidget& owner_;
    media::SlideRenderer slide_renderer_;
};

CueGridWidget::CueGridWidget(QWidget* parent) : QWidget(parent), grid_container_(new QWidget(this)) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    empty_scroll_ = new QScrollArea{this};
    empty_scroll_->setObjectName(QStringLiteral("presentation-empty-scroll"));
    empty_scroll_->setFrameShape(QFrame::NoFrame);
    empty_scroll_->setWidgetResizable(true);
    empty_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    empty_scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    empty_state_ = new QWidget{empty_scroll_};
    empty_state_->setObjectName(QStringLiteral("presentation-empty-state"));
    auto* empty_layout = new QVBoxLayout{empty_state_};
    empty_layout->setContentsMargins(24, 24, 24, 24);
    empty_layout->setSpacing(8);
    auto* title = new QLabel{QStringLiteral("Presentation"), empty_state_};
    title->setObjectName(QStringLiteral("presentation-title"));
    title->setAlignment(Qt::AlignCenter);
    auto* message = new QLabel{
        QStringLiteral("Add a Song or Sermon to the Service Order, or create content to begin."),
        empty_state_};
    message->setObjectName(QStringLiteral("presentation-empty-message"));
    message->setAlignment(Qt::AlignCenter);
    message->setWordWrap(true);
    empty_layout->addStretch(1);
    empty_layout->addWidget(title);
    empty_layout->addWidget(message);
    const auto add_action = [&](const QString& label, const QString& name, auto handler) {
        auto* button = new QPushButton{label, empty_state_};
        button->setObjectName(name);
        button->setMinimumHeight(34);
        button->setMaximumWidth(320);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QObject::connect(button, &QPushButton::clicked, this, handler);
        empty_layout->addWidget(button, 0, Qt::AlignHCenter);
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
    empty_scroll_->setWidget(empty_state_);
    outer->addWidget(empty_scroll_);

    grid_scroll_ = new QScrollArea{this};
    grid_scroll_->setObjectName(QStringLiteral("cue-grid-scroll"));
    grid_scroll_->setFrameShape(QFrame::NoFrame);
    grid_scroll_->setWidgetResizable(true);
    grid_scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    grid_scroll_->setWidget(grid_container_);
    grid_scroll_->viewport()->installEventFilter(this);
    outer->addWidget(grid_scroll_);
    grid_scroll_->setVisible(false);
    setMinimumWidth(220);

    apply_theme();
    QObject::connect(&ThemeManager::instance(), &ThemeManager::theme_changed, this,
                     [this](ThemeId) { apply_theme(); });
}

void CueGridWidget::apply_theme() {
    const auto& p = ThemeManager::instance().palette();
    setStyleSheet(
        QStringLiteral(
            "QWidget#cue-grid { background-color: %1; }"
            "QScrollArea#presentation-empty-scroll, QScrollArea#cue-grid-scroll,"
            "QWidget#presentation-empty-state {"
            "  background-color: %1;"
            "  color: %2;"
            "}"
            "QLabel#presentation-title {"
            "  color: %2;"
            "  font-size: 22px;"
            "  font-weight: 600;"
            "  background: transparent;"
            "}"
            "QLabel#presentation-empty-message {"
            "  color: %3;"
            "  background: transparent;"
            "}"
            "QWidget#cue-grid QPushButton {"
            "  background-color: %4;"
            "  border: 1px solid %5;"
            "  border-radius: 4px;"
            "  color: %2;"
            "  padding: 5px 10px;"
            "  font-weight: 600;"
            "}"
            "QWidget#cue-grid QPushButton:hover {"
            "  background-color: %6;"
            "  border-color: %7;"
            "}"
            "QWidget#cue-grid QPushButton:pressed {"
            "  background-color: %8;"
            "}")
            .arg(p.window_bg.name(),
                 p.text_primary.name(),
                 p.text_muted.name(),
                 p.button_bg.name(),
                 p.button_border.name(),
                 p.button_hover_bg.name(),
                 p.button_hover_border.name(),
                 p.button_pressed_bg.name()));
    if (grid_container_ != nullptr) {
        grid_container_->setStyleSheet(
            QStringLiteral("background-color: %1;").arg(p.window_bg.name()));
    }
    for (auto* card : cards_) {
        if (card != nullptr) {
            card->update();
        }
    }
    update();
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
    int screen_columns = 6;
    if (screen_width < 1600) {
        screen_columns = 3;
    } else if (screen_width < 2300) {
        screen_columns = 4;
    } else if (screen_width < 3200) {
        screen_columns = 5;
    }

    int viewport_columns = 6;
    if (viewport_width < 360) {
        viewport_columns = 1;
    } else if (viewport_width < 560) {
        viewport_columns = 2;
    } else if (viewport_width < 800) {
        viewport_columns = 3;
    } else if (viewport_width < 1100) {
        viewport_columns = 4;
    } else if (viewport_width < 1380) {
        viewport_columns = 5;
    }

    return std::min(screen_columns, viewport_columns);
}

void CueGridWidget::set_cues(std::vector<CueView> cues) {
    cues_ = std::move(cues);
    const bool empty = cues_.empty();
    empty_scroll_->setVisible(empty);
    grid_scroll_->setVisible(!empty);
    rebuild_cards();
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

bool CueGridWidget::eventFilter(QObject* watched, QEvent* event) {
    if (grid_scroll_ != nullptr && watched == grid_scroll_->viewport() &&
        event->type() == QEvent::Resize) {
        relayout_cards(false);
    }
    return QWidget::eventFilter(watched, event);
}

void CueGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    relayout_cards(false);
}

void CueGridWidget::rebuild_cards() {
    if (grid_container_ == nullptr) {
        return;
    }
    if (auto* old_layout = grid_container_->layout(); old_layout != nullptr) {
        while (auto* item = old_layout->takeAt(0)) {
            delete item;
        }
        delete old_layout;
    }
    for (auto* card : cards_) {
        card->deleteLater();
    }
    cards_.clear();
    cards_.reserve(cues_.size());

    for (const auto& cue : cues_) {
        auto* card = new CueCard{cue, *this};
        card->setProperty("cue-id", QString::fromStdString(cue.id.value));
        card->set_live(live_cue_.has_value() && cue.id == *live_cue_);
        cards_.push_back(card);
    }
    relayout_cards(true);
}

void CueGridWidget::relayout_cards(bool force) {
    if (grid_container_ == nullptr) {
        return;
    }
    const int viewport_width =
        (grid_scroll_ != nullptr && grid_scroll_->viewport()->width() > 0)
            ? grid_scroll_->viewport()->width()
            : width();
    const int screen_width = screen() == nullptr ? 1920 : screen()->size().width();
    const int columns = column_count_for_geometry(viewport_width, screen_width);
    if (!force && columns == current_columns_ && grid_container_->layout() != nullptr) {
        return;
    }
    current_columns_ = columns;

    if (auto* old_layout = grid_container_->layout(); old_layout != nullptr) {
        while (auto* item = old_layout->takeAt(0)) {
            delete item;
        }
        delete old_layout;
    }

    auto* layout = new QGridLayout(grid_container_);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    for (int col = 0; col < columns; ++col) {
        layout->setColumnStretch(col, 1);
    }
    for (std::size_t index = 0; index != cards_.size(); ++index) {
        const int row = static_cast<int>(index / static_cast<std::size_t>(columns));
        const int column = static_cast<int>(index % static_cast<std::size_t>(columns));
        layout->addWidget(cards_[index], row, column);
    }
    const int row_count =
        cards_.empty() ? 0 : static_cast<int>((cards_.size() + static_cast<std::size_t>(columns) - 1) / static_cast<std::size_t>(columns));
    layout->setRowStretch(row_count, 1);
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
