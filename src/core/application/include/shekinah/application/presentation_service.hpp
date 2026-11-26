#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

#include <optional>
#include <vector>

namespace shekinah::application {

class PresentationService {
public:
    void register_song(domain::Song song);
    void register_sermon(domain::SermonDeck deck);
    void reset() noexcept;

    domain::Status set_service_order(std::vector<domain::ServiceOrderItem> items);
    domain::Status select_service_item(const domain::ServiceOrderItemId& item_id);
    domain::Status take_live(const domain::ServiceOrderItemId& item_id, const domain::CueId& cue);
    domain::Status previous();
    domain::Status next();
    void clear() noexcept;
    void black() noexcept;

    [[nodiscard]] const domain::ProgramState& program() const noexcept { return program_; }
    [[nodiscard]] const std::optional<domain::ServiceOrderItem>& selected_item() const noexcept {
        return selected_item_;
    }
    domain::Result<std::optional<domain::BackgroundRef>> resolve_background(
        const domain::ContentRef& content, const domain::CueId& cue) const;

private:
    [[nodiscard]] const domain::ServiceOrderItem* find_item(
        const domain::ServiceOrderItemId& item_id) const noexcept;
    [[nodiscard]] const domain::Song* find_song(const domain::SongId& id) const noexcept;
    [[nodiscard]] const domain::SermonDeck* find_sermon(const domain::SermonId& id) const noexcept;
    [[nodiscard]] const domain::ServiceOrderItem* find_item_for_content(
        const domain::ContentRef& content) const noexcept;
    domain::Status move_live(int direction);

    std::vector<domain::Song> songs_;
    std::vector<domain::SermonDeck> sermons_;
    std::vector<domain::ServiceOrderItem> service_order_;
    std::optional<domain::ServiceOrderItem> selected_item_;
    domain::ProgramState program_{};
};

}  // namespace shekinah::application
