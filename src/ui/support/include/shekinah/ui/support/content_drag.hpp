#pragma once

#include "shekinah/domain/model.hpp"

#include <QMimeData>
#include <QString>

#include <optional>

namespace shekinah::ui {

inline QString content_mime_type() {
    return QStringLiteral("application/x-shekinah-content-ref");
}

inline QMimeData* make_content_mime_data(const domain::ContentRef& content) {
    auto* data = new QMimeData;
    const auto prefix = content.kind == domain::ContentKind::song ? "song:" : "sermon:";
    const auto& id = content.kind == domain::ContentKind::song
                         ? std::get<domain::SongId>(content.id).value
                         : std::get<domain::SermonId>(content.id).value;
    data->setData(content_mime_type(), QByteArray{prefix} + QByteArray::fromStdString(id));
    return data;
}

inline std::optional<domain::ContentRef> content_ref_from_mime_data(const QMimeData* data) {
    if (data == nullptr || !data->hasFormat(content_mime_type())) {
        return std::nullopt;
    }
    const auto value = data->data(content_mime_type());
    if (value.startsWith("song:")) {
        const auto id = value.sliced(5);
        return id.isEmpty() ? std::nullopt
                            : std::optional<domain::ContentRef>{domain::ContentRef::song(id.toStdString())};
    }
    if (value.startsWith("sermon:")) {
        const auto id = value.sliced(7);
        return id.isEmpty() ? std::nullopt
                            : std::optional<domain::ContentRef>{domain::ContentRef::sermon(id.toStdString())};
    }
    return std::nullopt;
}

}  // namespace shekinah::ui
