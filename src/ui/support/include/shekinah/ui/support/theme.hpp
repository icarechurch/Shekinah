#pragma once

#include "shekinah/ui/support/theme_types.hpp"

#include <QObject>
#include <QPalette>
#include <QString>

#include <vector>

class QSettings;

namespace shekinah::ui {

class ThemeManager final : public QObject {
    Q_OBJECT

public:
    static ThemeManager& instance();

    [[nodiscard]] ThemeId current_theme_id() const noexcept { return current_id_; }
    [[nodiscard]] ThemeId resolved_theme_id() const noexcept { return resolved_id_; }
    [[nodiscard]] const ThemePalette& palette() const noexcept { return current_palette_; }

    void set_theme(ThemeId id);
    bool set_theme_by_key(const QString& key);

    void load_from_settings(const QSettings& settings);
    void save_to_settings(QSettings& settings) const;

    [[nodiscard]] static const std::vector<ThemeInfo>& available_themes();
    [[nodiscard]] static ThemeInfo info_for(ThemeId id);
    [[nodiscard]] static ThemeId resolve_theme_id(ThemeId id);
    [[nodiscard]] static const ThemePalette& palette_for(ThemeId id);
    [[nodiscard]] static QPalette build_qpalette(const ThemePalette& p);
    [[nodiscard]] static QString dialog_stylesheet(const ThemePalette& p);

signals:
    void theme_changed(ThemeId id);

private:
    ThemeManager();
    void apply_resolved_theme(bool emit_signal);

    ThemeId current_id_{ThemeId::dark};
    ThemeId resolved_id_{ThemeId::dark};
    ThemePalette current_palette_;
};

}  // namespace shekinah::ui
