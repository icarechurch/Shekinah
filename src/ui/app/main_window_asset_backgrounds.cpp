#include "shekinah/ui/main_window.hpp"

#include "shekinah/application/song_backgrounds.hpp"
#include "shekinah/storage/content_repository.hpp"

#include <filesystem>

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

}  // namespace shekinah::ui
