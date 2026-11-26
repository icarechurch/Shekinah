#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/slide_insertion.hpp"
#include "shekinah/application/song_backgrounds.hpp"
#include "shekinah/storage/asset_importer.hpp"
#include "shekinah/storage/content_repository.hpp"
#include "shekinah/storage/project_store.hpp"
#include "shekinah/ui/library_dock_widget.hpp"
#include "shekinah/ui/operator_page.hpp"
#include "shekinah/ui/service_order_widget.hpp"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStringList>

#include <filesystem>
#include <utility>

namespace shekinah::ui {

QString ShekinahWindow::asset_name_for_cue(const domain::CueId& cue) const {
    if (!repository()) {
        return {};
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return {};
    }
    const auto background = presentation_.resolve_background(selected->content, cue);
    if (!background || !background->has_value()) {
        return {};
    }
    const auto asset = repository()->load_asset(background->value().asset);
    if (!asset) {
        return {};
    }
    const auto filename = std::filesystem::path{asset->relative_path}.filename().string();
    return QString::fromStdString(filename.empty() ? asset->relative_path : filename);
}

void ShekinahWindow::remove_asset_from_song(const domain::CueId& cue) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto background = presentation_.resolve_background(selected->content, cue);
    if (!background || !background->has_value()) {
        return;
    }
    auto song = repository()->load_song(std::get<domain::SongId>(selected->content.id));
    if (!song) {
        return;
    }
    application::remove_asset_from_song(*song, background->value().asset);
    const auto saved = repository()->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_cues();
    update_program_status();
}

void ShekinahWindow::choose_background(const domain::CueId& cue) {
    if (!repository()) {
        return;
    }
    const auto assets = repository()->list_assets();
    if (!assets || assets->empty()) {
        return;
    }
    QStringList labels;
    labels.reserve(static_cast<qsizetype>(assets->size()));
    for (const auto& asset : *assets) {
        labels.push_back(QString::fromStdString(asset.relative_path));
    }
    bool accepted = false;
    const auto selected = QInputDialog::getItem(
        this, QStringLiteral("Set Cue Background"), QStringLiteral("Asset"), labels, 0, false,
        &accepted);
    if (!accepted) {
        return;
    }
    const auto index = labels.indexOf(selected);
    if (index >= 0) {
        set_slide_background(cue, assets->at(static_cast<std::size_t>(index)).id);
    }
}

void ShekinahWindow::import_asset() {
    if (!store()) {
        return;
    }
    const auto path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Import Asset"), {},
        QStringLiteral(
            "Media (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.mp4);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    const auto imported = import_asset_file(path.toStdString());
    if (imported) {
        refresh_library();
        library_dock_->show_assets();
    } else {
        QMessageBox::warning(
            this, QStringLiteral("Asset import failed"), QString::fromStdString(imported.error().message));
    }
}

void ShekinahWindow::apply_asset_to_song(domain::ContentRef content, domain::AssetId asset) {
    if (!repository() || content.kind != domain::ContentKind::song) {
        return;
    }
    const auto asset_record = repository()->load_asset(asset);
    if (!asset_record) {
        return;
    }
    auto song = repository()->load_song(std::get<domain::SongId>(content.id));
    if (!song) {
        return;
    }
    application::set_default_background(
        *song, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
    const auto saved = repository()->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_cues();
    update_program_status();
}

void ShekinahWindow::apply_asset_to_slide(domain::CueId cue, domain::AssetId asset) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value() || selected->content.kind != domain::ContentKind::song) {
        return;
    }
    const auto asset_record = repository()->load_asset(asset);
    if (!asset_record) {
        return;
    }
    auto song = repository()->load_song(std::get<domain::SongId>(selected->content.id));
    if (!song) {
        return;
    }
    const auto changed = application::apply_background_from_slide(
        *song, cue, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
    if (!changed) {
        return;
    }
    const auto saved = repository()->save_song(*song);
    if (!saved) {
        return;
    }
    presentation_.register_song(*saved);
    refresh_cues();
    update_program_status();
}

void ShekinahWindow::insert_asset_at_slide_position(std::size_t index, domain::AssetId asset) {
    if (!repository()) {
        return;
    }
    const auto selected = presentation_.selected_item();
    if (!selected.has_value()) {
        return;
    }
    const auto asset_record = repository()->load_asset(asset);
    if (!asset_record) {
        return;
    }
    if (selected->content.kind == domain::ContentKind::song) {
        auto song = repository()->load_song(std::get<domain::SongId>(selected->content.id));
        if (!song) {
            return;
        }
        const auto inserted = application::insert_asset_slide(
            *song, index, domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
        if (!inserted) {
            return;
        }
        const auto saved = repository()->save_song(*song);
        if (!saved) {
            return;
        }
        presentation_.register_song(*saved);
        operator_page_->service_order_widget()->update_item_slide_count(
            selected->content, static_cast<int>(saved->slides.size()));
        refresh_cues();
        update_program_status();
        return;
    }

    auto sermon = repository()->load_deck(std::get<domain::SermonId>(selected->content.id));
    if (!sermon) {
        return;
    }
    const auto inserted = application::insert_asset_slide(*sermon, index, asset_record->id);
    if (!inserted) {
        return;
    }
    const auto saved = repository()->save_deck(*sermon);
    if (!saved) {
        return;
    }
    presentation_.register_sermon(*saved);
    operator_page_->service_order_widget()->update_item_slide_count(
        selected->content, static_cast<int>(saved->slides.size()));
    refresh_cues();
    update_program_status();
}

domain::Result<domain::AssetRecord> ShekinahWindow::import_asset_file(
    const std::filesystem::path& source) {
    if (!store()) {
        return domain::Result<domain::AssetRecord>::fail(
            {domain::ErrorCode::storage_failure, "Open a project before importing assets"});
    }
    return storage::AssetImporter{*store()}.import_file(source);
}


}  // namespace shekinah::ui

