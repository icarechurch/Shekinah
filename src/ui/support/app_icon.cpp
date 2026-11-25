#include "shekinah/ui/support/app_icon.hpp"

#include <QString>

static void ensure_shekinah_resources_initialized() {
    Q_INIT_RESOURCE(shekinah);
}

namespace shekinah::ui {

const QIcon& app_icon() {
    static const QIcon icon = [] {
        ensure_shekinah_resources_initialized();
        return QIcon{QStringLiteral(":/icons/shekinah.png")};
    }();
    return icon;
}

}  // namespace shekinah::ui
