#pragma once

#include "shekinah/media/slide_renderer.hpp"
#include "shekinah/ui/program_status_types.hpp"

class QPainter;
class QRect;
class QString;

namespace shekinah::ui {

void paint_program_slide_preview(QPainter& painter,
                                 const QRect& bounds,
                                 domain::ProgramMode mode,
                                 const QString& text,
                                 const QString& background_path,
                                 media::SlideRenderer& renderer);

}  // namespace shekinah::ui
