#include "shekinah/application/song_authoring.hpp"

#include <string>
#include <utility>
#include <vector>

namespace shekinah::application {

void normalize_song_cue_ids(domain::Song& song) {
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        song.slides[index].id = domain::CueId{
            song.id.value + "-slide-" + std::to_string(index)};
    }
}

SongReflowDocument reflow_document_for_song(const domain::Song& song) {
    std::string text;
    std::vector<SlideBreak> breaks;
    for (std::size_t index = 0; index != song.slides.size(); ++index) {
        if (index != 0) {
            breaks.push_back(SlideBreak{text.size(), song.slides[index].section});
            text.push_back('\n');
        }
        for (std::size_t line = 0; line != song.slides[index].lines.size(); ++line) {
            if (line != 0) {
                text.push_back('\n');
            }
            text += song.slides[index].lines[line];
        }
    }
    return SongReflowDocument{std::move(text), std::move(breaks)};
}

}  // namespace shekinah::application
