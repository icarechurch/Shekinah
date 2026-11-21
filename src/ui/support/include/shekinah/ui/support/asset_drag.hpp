#pragma once

#include "shekinah/domain/model.hpp"

#include <QMimeData>
#include <QString>

#include <optional>

namespace shekinah::ui {

inline QString asset_mime_type() {
    return QStringLiteral("application/x-shekinah-asset-id");
}

inline QMimeData* make_asset_mime_data(const domain::AssetId& asset) {
    auto* data = new QMimeData;
    data->setData(asset_mime_type(), QByteArray::fromStdString(asset.value));
    return data;
}

inline std::optional<domain::AssetId> asset_id_from_mime_data(const QMimeData* data) {
    if (data == nullptr || !data->hasFormat(asset_mime_type())) {
        return std::nullopt;
    }
    const auto value = data->data(asset_mime_type());
    if (value.isEmpty()) {
        return std::nullopt;
    }
    return domain::AssetId{value.toStdString()};
}

}  // namespace shekinah::ui
