#pragma once

#include <QColor>
#include <QIcon>

namespace shekinah::ui::program_status {

enum class IconGlyph {
    clear_all,
    slide,
    media_image,
    blackout,
    display,
    sliders,
    chevron_down,
    previous,
    next,
    clear_layer,
    layers,
    list,
    plus,
    arrow_right,
};

[[nodiscard]] QIcon make_glyph_icon(IconGlyph glyph, const QColor& color, int size = 18);

}  // namespace shekinah::ui::program_status
