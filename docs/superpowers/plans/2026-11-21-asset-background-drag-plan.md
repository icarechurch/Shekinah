# Asset Thumbnail Drag-to-Background Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the text-only asset catalog with thumbnail tiles and persist drag-and-drop backgrounds for Songs, Song slides, and their forward slide ranges.

**Architecture:** Keep the existing `AssetRecord`, `Song::default_background`, and `Slide::background` data model. Add a shared Qt MIME contract, make the asset library and presentation widgets report typed drag/drop callbacks, and keep all Song mutation and repository persistence in `ShekinahWindow`, with forward-range mutation isolated in a testable application helper.

**Tech Stack:** C++20, Qt 6.7 Widgets/Test, Catch2 3, CMake/Ninja, SQLite repository already used by the project.

## Global Constraints

- CI gates are blocking: preserve the existing Windows UCRT64 debug/release configure, build, test, and N4000 benchmark workflow.
- Use the existing `Song::default_background` and `Slide::background` fields; do not add a database migration.
- A Song-row drop changes only the Song default; a Song-slide drop changes the dropped slide and all later slides.
- Sermon targets do not accept asset background drops.
- The existing right-click `Set background` action remains single-slide behavior.
- Keep all existing tests and assertions; do not use focused tests, skipped tests, swallowed failures, or disabled checks.
- Run focused checks after each slice, then the complete debug and release gates before completion.
- Deliver at least 20 meaningful commits; no empty commits or commits that only rearrange whitespace.

---

## File map

Create `src/ui/support/asset_drag.hpp` for the shared MIME type and `AssetId` payload helpers.

Create `src/core/application/include/shekinah/application/song_backgrounds.hpp` and `src/core/application/song_backgrounds.cpp` for pure in-memory Song background mutations.

Modify `src/ui/widgets/library_dock/library_dock_widget.cpp` to use a local draggable `QListWidget` subclass, icon-mode asset tiles, bounded image thumbnails, video fallback icons, and compact filename labels.

Modify `src/ui/widgets/service_order/include/shekinah/ui/service_order_widget.hpp` and `src/ui/widgets/service_order/service_order_widget.cpp` to accept asset drops and report the targeted `ContentRef`.

Modify `src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp` and `src/ui/widgets/cue_grid/cue_grid_widget.cpp` to accept asset drops on individual cue cards and report the targeted `CueId`.

Modify `src/ui/pages/operator/include/shekinah/ui/operator_page.hpp` and `src/ui/pages/operator/operator_page.cpp` to forward the two typed drop callbacks.

Modify `src/ui/app/include/shekinah/ui/main_window.hpp` and `src/ui/app/main_window.cpp` to wire callbacks, validate target assets, mutate Songs, save them, register them with `PresentationService`, and refresh cues/live preview.

Modify `src/core/application/CMakeLists.txt` to compile the Song background helper.

Modify `tests/CMakeLists.txt`, `tests/application/presentation_service_test.cpp` or a new `tests/application/song_backgrounds_test.cpp`, `tests/ui/library_dock_test.cpp`, `tests/ui/service_order_widget_test.cpp`, `tests/ui/cue_grid_test.cpp`, `tests/ui/operator_page_test.cpp`, and `tests/ui/project_session_test.cpp` for the focused and integration coverage.

Create `docs/superpowers/specs/2026-11-21-asset-background-drag-design.md` (already committed) and keep the approved behavior as the source of truth.

---

### Task 1: Add the shared asset-drag contract

**Files:**
- Create: `src/ui/support/asset_drag.hpp`
- Test: `tests/ui/library_dock_test.cpp`
- Modify: `tests/CMakeLists.txt` only if the test file needs a new source entry; it is already part of `shekinah_ui_tests`.

**Interfaces:**
- Produces `shekinah::ui::asset_mime_type()` returning `QString`.
- Produces `shekinah::ui::make_asset_mime_data(const domain::AssetId&)` returning `QMimeData*` owned by the drag operation.
- Produces `shekinah::ui::asset_id_from_mime_data(const QMimeData*)` returning `std::optional<domain::AssetId>`.

- [ ] **Step 1: Write the failing test**

Add this test to `tests/ui/library_dock_test.cpp`:

```cpp
#include <QMimeData>

#include "shekinah/ui/support/asset_drag.hpp"

TEST_CASE("asset drag payload round-trips an AssetId") {
    ensure_qapplication();
    const auto payload = shekinah::ui::make_asset_mime_data(AssetId{"asset-42"});

    REQUIRE(payload != nullptr);
    REQUIRE(payload->hasFormat(shekinah::ui::asset_mime_type().toUtf8().constData()));
    const auto parsed = shekinah::ui::asset_id_from_mime_data(payload);
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->value == "asset-42");
    delete payload;
}
```

- [ ] **Step 2: Run the failing test**

Run:

```text
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug -R "asset drag payload" --output-on-failure
```

Expected: compilation fails because `shekinah/ui/support/asset_drag.hpp` does not exist.

- [ ] **Step 3: Write the minimal implementation**

Create the header with this implementation:

```cpp
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
```

- [ ] **Step 4: Run the focused test**

Run the same build and `ctest` command. Expected: the new test passes and the existing UI test executable remains green.

- [ ] **Step 5: Commit**

```text
git add src/ui/support/asset_drag.hpp tests/ui/library_dock_test.cpp
git commit -m "feat: add asset drag payload contract"
```

### Task 2: Switch the Assets list to a tile-grid structure

**Files:**
- Modify: `src/ui/widgets/library_dock/library_dock_widget.cpp`
- Test: `tests/ui/library_dock_test.cpp`

**Interfaces:**
- Keeps the existing `assets-list` object name and `add_asset` API.
- The asset list exposes Qt `IconMode`, wrapping, static movement, and drag-only configuration.

- [ ] **Step 1: Write the failing test**

Add:

```cpp
#include <QListView>

TEST_CASE("library presents assets as a wrapping tile grid") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-1"}, AssetKind::image, "hash", "one.png", 100});

    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list != nullptr);
    REQUIRE(list->viewMode() == QListView::IconMode);
    REQUIRE(list->flow() == QListView::LeftToRight);
    REQUIRE(list->isWrapping());
    REQUIRE(list->movement() == QListView::Static);
    REQUIRE(list->dragEnabled());
    REQUIRE(list->dragDropMode() == QAbstractItemView::DragOnly);
}
```

- [ ] **Step 2: Run it and confirm the expected failure**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "library presents assets as a wrapping tile grid" --output-on-failure
```

Expected: the test fails because the current list is in the default list mode and dragging is disabled.

- [ ] **Step 3: Implement the list configuration**

In `LibraryDockWidget` construction, after `assets_` is created, add:

```cpp
assets_->setViewMode(QListView::IconMode);
assets_->setFlow(QListView::LeftToRight);
assets_->setWrapping(true);
assets_->setResizeMode(QListView::Adjust);
assets_->setMovement(QListView::Static);
assets_->setSelectionMode(QAbstractItemView::SingleSelection);
assets_->setDragEnabled(true);
assets_->setDragDropMode(QAbstractItemView::DragOnly);
assets_->setSpacing(10);
assets_->setIconSize(QSize{168, 94});
assets_->setGridSize(QSize{192, 126});
assets_->setUniformItemSizes(true);
```

Add `<QListView>` and keep `<QAbstractItemView>` available through the Qt widget includes.

- [ ] **Step 4: Run the focused test**

Run the focused test and `ctest --test-dir build/windows-ucrt64-debug -R "library (tabs|presents assets)" --output-on-failure`. Expected: both tile behavior and existing tab retention pass.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/library_dock/library_dock_widget.cpp tests/ui/library_dock_test.cpp
git commit -m "feat: present assets in a thumbnail grid"
```

### Task 3: Render bounded image thumbnails

**Files:**
- Modify: `src/ui/widgets/library_dock/library_dock_widget.cpp`
- Test: `tests/ui/library_dock_test.cpp`

**Interfaces:**
- Image assets keep `QImageReader::setAutoTransform(true)` and a maximum thumbnail size of `168x94`.
- Asset labels use the filename component of `relative_path`.

- [ ] **Step 1: Write the failing test**

Extend the existing thumbnail test:

```cpp
REQUIRE(list->item(0)->text() == QStringLiteral("slide.png"));
REQUIRE(list->item(0)->icon().availableSizes().front().width() <= 168);
REQUIRE(list->item(0)->icon().availableSizes().front().height() <= 94);
```

- [ ] **Step 2: Run the test and confirm the failure**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "library creates bounded thumbnails" --output-on-failure
```

Expected: the label is currently `slide.png` only for the fixture but icon dimensions are governed by the old list setup, so the new size assertion fails once the tile dimensions differ.

- [ ] **Step 3: Implement bounded thumbnails and filename labels**

In `add_asset`, derive the display label and scale the reader:

```cpp
const auto filename = std::filesystem::path{asset.relative_path}.filename().string();
auto* item = new QListWidgetItem{QString::fromStdString(filename.empty() ? asset.relative_path : filename)};
item->setData(kContentIdRole, QString::fromStdString(asset.id.value));
item->setToolTip(QStringLiteral("%1\n%2 bytes")
                    .arg(QString::fromStdString(asset.relative_path))
                    .arg(static_cast<qulonglong>(asset.size_bytes)));
if (asset.kind == domain::AssetKind::image && !asset_root_.empty()) {
    QImageReader reader{QString::fromStdString((asset_root_ / asset.relative_path).string())};
    reader.setAutoTransform(true);
    const auto source_size = reader.size();
    if (source_size.isValid()) {
        reader.setScaledSize(source_size.scaled(168, 94, Qt::KeepAspectRatio));
    }
    const auto image = reader.read();
    if (!image.isNull()) {
        item->setIcon(QIcon{QPixmap::fromImage(image)});
    }
}
```

- [ ] **Step 4: Run the focused test**

Run the build and `ctest --test-dir build/windows-ucrt64-debug -R "library creates bounded thumbnails" --output-on-failure`. Expected: the thumbnail and filename assertions pass.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/library_dock/library_dock_widget.cpp tests/ui/library_dock_test.cpp
git commit -m "feat: render bounded asset thumbnails"
```

### Task 4: Add the video fallback tile

**Files:**
- Modify: `src/ui/widgets/library_dock/library_dock_widget.cpp`
- Test: `tests/ui/library_dock_test.cpp`

**Interfaces:**
- Video assets remain draggable and show a non-empty Qt media icon.

- [ ] **Step 1: Write the failing test**

Add:

```cpp
TEST_CASE("library presents video assets with a media fallback icon") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-video"}, AssetKind::video, "hash", "loop.mp4", 200});

    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list != nullptr);
    REQUIRE(list->item(0)->text() == QStringLiteral("loop.mp4"));
    REQUIRE_FALSE(list->item(0)->icon().isNull());
}
```

- [ ] **Step 2: Run the test**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "video assets" --output-on-failure
```

Expected: FAIL because the video branch has not yet assigned the icon in the tile-grid implementation.

- [ ] **Step 3: Implement the fallback**

Keep the existing Qt style lookup after the image branch:

```cpp
} else if (asset.kind == domain::AssetKind::video) {
    item->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}
```

- [ ] **Step 4: Run the focused test**

Run the build and the same `ctest` filter. Expected: PASS.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/library_dock/library_dock_widget.cpp tests/ui/library_dock_test.cpp
git commit -m "feat: show media icons for video assets"
```

### Task 5: Preserve labels, filtering, and asset identity in tile mode

**Files:**
- Modify: `src/ui/widgets/library_dock/library_dock_widget.cpp`
- Test: `tests/ui/library_dock_test.cpp`

**Interfaces:**
- Filtering continues to inspect the visible filename label.
- Every tile retains the full `AssetId` in the existing user role.

- [ ] **Step 1: Write the failing test**

Add:

```cpp
TEST_CASE("asset tiles preserve ids and filter by filename") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-one"}, AssetKind::image, "h1", "media/one.png", 10});
    dock.add_asset(AssetRecord{AssetId{"asset-two"}, AssetKind::image, "h2", "media/two.png", 20});
    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list->item(0)->data(Qt::UserRole + 1).toString() == QStringLiteral("asset-one"));

    dock.set_search_text(QStringLiteral("two"));
    REQUIRE(list->item(0)->isHidden());
    REQUIRE_FALSE(list->item(1)->isHidden());
}
```

- [ ] **Step 2: Run it and observe the failure**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "asset tiles preserve" --output-on-failure
```

Expected: FAIL if the filename label or role assignment is not preserved through the tile conversion.

- [ ] **Step 3: Implement the identity/label boundary**

Use the filename-derived label only for display, keep the full path in the tooltip, and continue assigning:

```cpp
item->setData(kContentIdRole, QString::fromStdString(asset.id.value));
```

Keep `apply_filter()` unchanged so it filters the item text and does not delete tiles.

- [ ] **Step 4: Run the focused test**

Run the build and `ctest --test-dir build/windows-ucrt64-debug -R "asset tiles preserve|library search" --output-on-failure`. Expected: both new and existing filtering tests pass.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/library_dock/library_dock_widget.cpp tests/ui/library_dock_test.cpp
git commit -m "test: preserve asset tile identity and filtering"
```

### Task 6: Make asset tiles the drag source

**Files:**
- Modify: `src/ui/widgets/library_dock/library_dock_widget.cpp`
- Test: `tests/ui/library_dock_test.cpp`

**Interfaces:**
- A single selected asset item produces the shared `application/x-shekinah-asset-id` payload.
- Items without a valid id produce `nullptr` from `mimeData`.

- [ ] **Step 1: Write the failing test**

Add a library-level assertion that the asset item keeps its id while the list is configured for drag-only operation:

```cpp
TEST_CASE("asset tiles retain their id for dragging") {
    ensure_qapplication();
    LibraryDockWidget dock;
    dock.add_asset(AssetRecord{AssetId{"asset-drag"}, AssetKind::image, "hash", "drag.png", 1});
    auto* list = dock.findChild<QListWidget*>(QStringLiteral("assets-list"));
    REQUIRE(list->dragEnabled());
    REQUIRE(list->dragDropMode() == QAbstractItemView::DragOnly);
    REQUIRE(list->item(0)->data(Qt::UserRole + 1).toString() == QStringLiteral("asset-drag"));
}
```

- [ ] **Step 2: Run the failing test**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "asset list emits" --output-on-failure
```

Expected: FAIL because the draggable list subclass and MIME override do not exist.

- [ ] **Step 3: Implement the drag source**

Create a local `AssetListWidget` subclass in `library_dock_widget.cpp`:

```cpp
class AssetListWidget final : public QListWidget {
public:
    using QListWidget::QListWidget;

protected:
    QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override {
        if (items.size() != 1 || items.front() == nullptr) {
            return nullptr;
        }
        const auto id = items.front()->data(kContentIdRole).toString();
        return id.isEmpty() ? nullptr : make_asset_mime_data(domain::AssetId{id.toStdString()});
    }
};
```

Construct `AssetListWidget` instead of `QListWidget`. The shared MIME round-trip test from Task 1 verifies serialization; this task verifies that the actual asset list is configured as the drag source.

- [ ] **Step 4: Run the focused test**

Run the build and `ctest --test-dir build/windows-ucrt64-debug -R "asset tiles retain|asset drag payload" --output-on-failure`. Expected: PASS.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/library_dock/include/shekinah/ui/library_dock_widget.hpp src/ui/widgets/library_dock/library_dock_widget.cpp tests/ui/library_dock_test.cpp
git commit -m "feat: make asset tiles draggable"
```

### Task 7: Add and implement Service Order asset drops

**Files:**
- Modify: `src/ui/widgets/service_order/include/shekinah/ui/service_order_widget.hpp`
- Modify: `src/ui/widgets/service_order/service_order_widget.cpp`
- Modify: `tests/ui/service_order_widget_test.cpp`

**Interfaces:**
- Add `using AssetDropHandler = std::function<void(domain::ContentRef, domain::AssetId)>;`.
- Add `set_asset_drop_handler(AssetDropHandler handler)`.
- Add private `handle_asset_drop(const domain::AssetId&, int row)`.

- [ ] **Step 1: Write the failing test**

Add Qt event headers and test the real list drop path:

```cpp
TEST_CASE("service order reports an asset dropped on a Song row") {
    ensure_qapplication();
    ServiceOrderWidget widget;
    widget.set_items({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-1"), 0},
        QStringLiteral("Song")}});
    ContentRef target = ContentRef::sermon("unset");
    AssetId asset{"unset"};
    widget.set_asset_drop_handler([&](ContentRef content, AssetId dropped) {
        target = std::move(content);
        asset = std::move(dropped);
    });

    auto* list = widget.findChild<QListWidget*>(QStringLiteral("service-order-list"));
    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-1"});
    QDropEvent event{list->visualItemRect(list->item(0)).center(), Qt::CopyAction, mime,
                     Qt::LeftButton, Qt::NoModifier, Qt::CopyAction};
    QApplication::sendEvent(list, &event);

    REQUIRE(event.isAccepted());
    REQUIRE(target == ContentRef::song("song-1"));
    REQUIRE(asset == AssetId{"asset-1"});
    delete mime;
}
```

- [ ] **Step 2: Run the failing test**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "service order reports an asset" --output-on-failure
```

Expected: FAIL because the list does not accept asset drop events.

- [ ] **Step 3: Add the callback and drop-enabled list**

Add to the header:

```cpp
using AssetDropHandler = std::function<void(domain::ContentRef, domain::AssetId)>;
void set_asset_drop_handler(AssetDropHandler handler) { asset_drop_handler_ = std::move(handler); }
```

Store `AssetDropHandler asset_drop_handler_;` privately. Create a local `ServiceOrderListWidget`, set `setAcceptDrops(true)`, and implement `dragEnterEvent`, `dragMoveEvent`, and `dropEvent` using `asset_id_from_mime_data`. On a valid row, call the parent handler with `items_[row].item.content` and the parsed asset id, then call `acceptProposedAction()`.

- [ ] **Step 4: Run the focused test**

Run the build and the same `ctest` filter. Expected: PASS for the real row-drop contract.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/service_order/include/shekinah/ui/service_order_widget.hpp tests/ui/service_order_widget_test.cpp
git commit -m "feat: define service-order asset drop callbacks"
```

### Task 8: Implement Service Order row drops

**Files:**
- Modify: `src/ui/widgets/service_order/service_order_widget.cpp`
- Modify: `src/ui/widgets/service_order/include/shekinah/ui/service_order_widget.hpp`
- Test: `tests/ui/service_order_widget_test.cpp`

**Interfaces:**
- The `service-order-list` accepts only valid asset MIME payloads.
- A drop on a row reports that row's `ContentRef`; a drop outside rows is ignored.

- [ ] **Step 1: Write the failing event test**

Add Qt event headers and this test:

```cpp
TEST_CASE("service order reports an asset dropped on a Song row") {
    ensure_qapplication();
    ServiceOrderWidget widget;
    widget.set_items({ServiceOrderView{
        ServiceOrderItem{ServiceOrderItemId{"item-1"}, ContentRef::song("song-1"), 0},
        QStringLiteral("Goodness")}});
    ContentRef target = ContentRef::sermon("unset");
    AssetId asset{"unset"};
    widget.set_asset_drop_handler([&](ContentRef content, AssetId dropped) {
        target = std::move(content);
        asset = std::move(dropped);
    });

    auto* list = widget.findChild<QListWidget*>(QStringLiteral("service-order-list"));
    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-1"});
    QDropEvent event{list->visualItemRect(list->item(0)).center(), Qt::CopyAction, mime,
                     Qt::LeftButton, Qt::NoModifier, Qt::CopyAction};
    QApplication::sendEvent(list, &event);

    REQUIRE(event.isAccepted());
    REQUIRE(target == ContentRef::song("song-1"));
    REQUIRE(asset == AssetId{"asset-1"});
    delete mime;
}
```

- [ ] **Step 2: Run it and observe the failure**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "service order reports an asset" --output-on-failure
```

Expected: FAIL because the list does not accept drop events.

- [ ] **Step 3: Implement the drop-enabled list**

Create a local `ServiceOrderListWidget` subclass with an `AssetDropHandler` callback. Its event methods must use `asset_id_from_mime_data`, call `acceptProposedAction()` for valid payloads, and find the row with `itemAt(event->position().toPoint())`. On a valid row, call the parent widget's handler with `items_[row].item.content` and the parsed asset id.

The essential drop path is:

```cpp
const auto asset = asset_id_from_mime_data(event->mimeData());
auto* item = itemAt(event->position().toPoint());
if (!asset.has_value() || item == nullptr) {
    event->ignore();
    return;
}
const auto row = row(item);
if (row < 0 || row >= static_cast<int>(items_.size()) || !asset_drop_handler_) {
    event->ignore();
    return;
}
asset_drop_handler_(items_[static_cast<std::size_t>(row)].item.content, *asset);
event->acceptProposedAction();
```

Set `setAcceptDrops(true)` on the list, and keep selection/move behavior unchanged.

- [ ] **Step 4: Run focused tests**

Run:

```text
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug -R "service order (reports an asset|presents reusable|reports requested moves)" --output-on-failure
```

Expected: all selected Service Order tests pass.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/service_order/include/shekinah/ui/service_order_widget.hpp src/ui/widgets/service_order/service_order_widget.cpp tests/ui/service_order_widget_test.cpp
git commit -m "feat: accept asset drops on service-order rows"
```

### Task 9: Add and implement Cue Card asset drops

**Files:**
- Modify: `src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp`
- Modify: `src/ui/widgets/cue_grid/cue_grid_widget.cpp`
- Modify: `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- Add `using AssetDropHandler = std::function<void(domain::CueId, domain::AssetId)>;`.
- Add `set_asset_drop_handler(AssetDropHandler handler)`.
- Add private `handle_asset_drop(const domain::CueId&, const domain::AssetId&)`.

- [ ] **Step 1: Write the failing test**

Add an event test against the actual `CueCard`:

```cpp
TEST_CASE("cue grid reports the cue id for an asset drop") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({CueView{CueId{"cue-1"}, QStringLiteral("Verse"), QStringLiteral("Line"), 1}});
    CueId target{"unset"};
    AssetId asset{"unset"};
    grid.set_asset_drop_handler([&](CueId cue, AssetId dropped) {
        target = std::move(cue);
        asset = std::move(dropped);
    });

    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-cue-1"));
    REQUIRE(card != nullptr);
    auto* mime = shekinah::ui::make_asset_mime_data(AssetId{"asset-1"});
    QDropEvent event{card->rect().center(), Qt::CopyAction, mime,
                     Qt::LeftButton, Qt::NoModifier, Qt::CopyAction};
    QApplication::sendEvent(card, &event);

    REQUIRE(event.isAccepted());
    REQUIRE(target == CueId{"cue-1"});
    REQUIRE(asset == AssetId{"asset-1"});
    delete mime;
}
```

- [ ] **Step 2: Run the failing test**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "cue grid reports the cue" --output-on-failure
```

Expected: FAIL because `CueCard` currently handles only mouse clicks.

- [ ] **Step 3: Add the callback and card event handlers**

Add the type, setter, and private storage. Include `QDragEnterEvent` and `QDropEvent`; set `setAcceptDrops(true)` in the card constructor; and implement the event path:

```cpp
void dropEvent(QDropEvent* event) override {
    const auto asset = asset_id_from_mime_data(event->mimeData());
    if (!asset.has_value()) {
        event->ignore();
        return;
    }
    owner_.handle_asset_drop(cue_.id, *asset);
    event->acceptProposedAction();
}
```

`dragEnterEvent` and `dragMoveEvent` accept only payloads recognized by `asset_id_from_mime_data`.

- [ ] **Step 4: Run the focused test**

Run the build and the same filter. Expected: PASS.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp tests/ui/cue_grid_test.cpp
git commit -m "feat: define cue-card asset drop callbacks"
```

### Task 10: Harden Cue Card drop rejection

**Files:**
- Modify: `src/ui/widgets/cue_grid/cue_grid_widget.cpp`
- Modify: `src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp`
- Test: `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- Each `CueCard` accepts a valid asset MIME payload.
- A drop invokes the grid callback with that card's `CueId`.
- Invalid payloads are ignored and do not invoke the callback.

- [ ] **Step 1: Write the failing rejection test**

Add:

```cpp
TEST_CASE("cue card ignores unsupported asset drops") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({CueView{CueId{"cue-1"}, QStringLiteral("Verse"), QStringLiteral("Line"), 1}});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-cue-1"));
    REQUIRE(card != nullptr);
    QMimeData unsupported;
    QDropEvent event{card->rect().center(), Qt::CopyAction, &unsupported,
                     Qt::LeftButton, Qt::NoModifier, Qt::CopyAction};
    QApplication::sendEvent(card, &event);

    REQUIRE_FALSE(event.isAccepted());
}
```

- [ ] **Step 2: Run it and observe the failure**

Run the new test filter. Expected: FAIL if the card accepts unsupported MIME data.

- [ ] **Step 3: Implement card drag/drop events**

In `CueCard`, keep this guard before invoking the owner callback:

```cpp
void dragEnterEvent(QDragEnterEvent* event) override {
    if (asset_id_from_mime_data(event->mimeData()).has_value()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void dropEvent(QDropEvent* event) override {
    const auto asset = asset_id_from_mime_data(event->mimeData());
    if (!asset.has_value()) {
        event->ignore();
        return;
    }
    owner_.handle_asset_drop(cue_.id, *asset);
    event->acceptProposedAction();
}
```

Call `setAcceptDrops(true)` in the card constructor. Keep the current left/right mouse handlers unchanged.

- [ ] **Step 4: Run focused tests**

Run:

```text
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug -R "cue card (accepts|exposes|paints|footer)" --output-on-failure
```

Expected: new drop coverage and existing cue-card coverage pass.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp src/ui/widgets/cue_grid/cue_grid_widget.cpp tests/ui/cue_grid_test.cpp
git commit -m "feat: accept asset drops on cue cards"
```

### Task 11: Forward drop callbacks through OperatorPage

**Files:**
- Modify: `src/ui/pages/operator/include/shekinah/ui/operator_page.hpp`
- Modify: `src/ui/pages/operator/operator_page.cpp`
- Test: `tests/ui/operator_page_test.cpp`

**Interfaces:**
- Add `set_service_order_asset_drop_handler(std::function<void(domain::ContentRef, domain::AssetId)>)`.
- Add `set_cue_asset_drop_handler(std::function<void(domain::CueId, domain::AssetId)>)`.

- [ ] **Step 1: Write the failing test**

Extend the operator forwarding test with two callbacks and invoke the widget test seams. Assert that the page forwards `ContentRef::song("song-1")` and `CueId{"cue-1"}` with `AssetId{"asset-1"}`.

- [ ] **Step 2: Run the failing test**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "operator page forwards.*asset" --output-on-failure
```

Expected: FAIL because the forwarding APIs do not exist.

- [ ] **Step 3: Implement forwarding**

Add the two methods to the header and implement them by calling:

```cpp
service_order_->set_asset_drop_handler(std::move(handler));
cue_grid_->set_asset_drop_handler(std::move(handler));
```

Do not add repository or domain mutation logic to `OperatorPage`.

- [ ] **Step 4: Run focused tests**

Run the build and the operator page test filter. Expected: PASS with all existing operator tests.

- [ ] **Step 5: Commit**

```text
git add src/ui/pages/operator/include/shekinah/ui/operator_page.hpp src/ui/pages/operator/operator_page.cpp tests/ui/operator_page_test.cpp
git commit -m "feat: forward presentation asset drops"
```

### Task 12: Add forward Song-slide background mutation

**Files:**
- Create: `src/core/application/include/shekinah/application/song_backgrounds.hpp`
- Create: `src/core/application/song_backgrounds.cpp`
- Modify: `src/core/application/CMakeLists.txt`
- Create: `tests/application/song_backgrounds_test.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- `domain::Status apply_background_from_slide(domain::Song&, const domain::CueId&, domain::BackgroundRef)`.

- [ ] **Step 1: Write the failing tests**

Create `tests/application/song_backgrounds_test.cpp`:

```cpp
#include <catch2/catch_test_macros.hpp>

#include "shekinah/application/song_backgrounds.hpp"

using namespace shekinah::application;
using namespace shekinah::domain;

TEST_CASE("slide background assignment applies from the selected slide forward") {
    Song song{SongId{"song"}, "Song", std::nullopt,
              {Slide{CueId{"one"}, "Verse", {"One"}, std::nullopt},
               Slide{CueId{"two"}, "Chorus", {"Two"}, BackgroundRef{AssetId{"old"}}},
               Slide{CueId{"three"}, "Bridge", {"Three"}, std::nullopt}}};

    REQUIRE(apply_background_from_slide(song, CueId{"two"},
                                        BackgroundRef{AssetId{"new"}, FitMode::cover}));
    REQUIRE_FALSE(song.slides[0].background.has_value());
    REQUIRE(song.slides[1].background->asset == AssetId{"new"});
    REQUIRE(song.slides[2].background->asset == AssetId{"new"});
}

TEST_CASE("slide background assignment rejects an unknown cue without mutation") {
    Song song{SongId{"song"}, "Song", std::nullopt,
              {Slide{CueId{"one"}, "Verse", {"One"}, std::nullopt}}};

    REQUIRE_FALSE(apply_background_from_slide(song, CueId{"missing"},
                                              BackgroundRef{AssetId{"new"}}));
    REQUIRE_FALSE(song.slides[0].background.has_value());
}
```

- [ ] **Step 2: Run and confirm RED**

Run:

```text
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug -R "slide background assignment" --output-on-failure
```

Expected: compilation fails because the application helper does not exist.

- [ ] **Step 3: Implement the minimal helper**

Header:

```cpp
#pragma once

#include "shekinah/domain/model.hpp"
#include "shekinah/domain/result.hpp"

namespace shekinah::application {

domain::Status apply_background_from_slide(domain::Song& song,
                                           const domain::CueId& start,
                                           domain::BackgroundRef background);

}  // namespace shekinah::application
```

Implementation:

```cpp
#include "shekinah/application/song_backgrounds.hpp"

#include <algorithm>

namespace shekinah::application {

domain::Status apply_background_from_slide(domain::Song& song,
                                           const domain::CueId& start,
                                           domain::BackgroundRef background) {
    const auto first = std::find_if(song.slides.begin(), song.slides.end(), [&](const auto& slide) {
        return slide.id == start;
    });
    if (first == song.slides.end()) {
        return domain::Status::fail({domain::ErrorCode::not_found, "Song cue was not found"});
    }
    for (auto slide = first; slide != song.slides.end(); ++slide) {
        slide->background = background;
    }
    return domain::success();
}

}  // namespace shekinah::application
```

Add `song_backgrounds.cpp` to `shekinah_application` and the new test source to `shekinah_application_tests`.

- [ ] **Step 4: Run focused application tests**

Run:

```text
cmake --preset windows-ucrt64-debug
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug -R "slide background assignment|background resolution" --output-on-failure
```

Expected: both new helper tests and existing background resolution tests pass.

- [ ] **Step 5: Commit**

```text
git add src/core/application/include/shekinah/application/song_backgrounds.hpp src/core/application/song_backgrounds.cpp src/core/application/CMakeLists.txt tests/application/song_backgrounds_test.cpp tests/CMakeLists.txt
git commit -m "feat: apply Song backgrounds across slide ranges"
```

### Task 13: Add Song default-background mutation

**Files:**
- Modify: `src/core/application/include/shekinah/application/song_backgrounds.hpp`
- Modify: `src/core/application/song_backgrounds.cpp`
- Modify: `tests/application/song_backgrounds_test.cpp`

**Interfaces:**
- `void set_default_background(domain::Song&, domain::BackgroundRef)`.

- [ ] **Step 1: Write the failing test**

Add:

```cpp
TEST_CASE("Song default background assignment leaves slide overrides unchanged") {
    Song song{SongId{"song"}, "Song", std::nullopt,
              {Slide{CueId{"one"}, "Verse", {"One"}, BackgroundRef{AssetId{"slide"}}}}};

    set_default_background(song, BackgroundRef{AssetId{"default"}, FitMode::contain});

    REQUIRE(song.default_background->asset == AssetId{"default"});
    REQUIRE(song.default_background->fit == FitMode::contain);
    REQUIRE(song.slides[0].background->asset == AssetId{"slide"});
}
```

- [ ] **Step 2: Run it and confirm the failure**

Run the focused application filter. Expected: compilation fails because `set_default_background` is not declared.

- [ ] **Step 3: Implement the setter**

Add the declaration and this implementation:

```cpp
void set_default_background(domain::Song& song, domain::BackgroundRef background) {
    song.default_background = std::move(background);
}
```

Include `<utility>` in the implementation.

- [ ] **Step 4: Run focused tests**

Run the build and `ctest --test-dir build/windows-ucrt64-debug -R "Song default background|slide background assignment|background resolution" --output-on-failure`. Expected: PASS.

- [ ] **Step 5: Commit**

```text
git add src/core/application/include/shekinah/application/song_backgrounds.hpp src/core/application/song_backgrounds.cpp tests/application/song_backgrounds_test.cpp
git commit -m "feat: assign Song default backgrounds"
```

### Task 14: Add main-window Song mutation boundaries

**Files:**
- Modify: `src/ui/app/include/shekinah/ui/main_window.hpp`
- Modify: `src/ui/app/main_window.cpp`
- Test: `tests/ui/main_window_test.cpp`

**Interfaces:**
- Add private `void apply_asset_to_song(domain::ContentRef, domain::AssetId)`.
- Add private `void apply_asset_to_slide(domain::CueId, domain::AssetId)`.
- Keep public `set_slide_background` as the existing single-slide API.

- [ ] **Step 1: Write failing boundary tests**

Add tests that open a project, import an image, import a Song, append/select the Song, invoke the new test-visible drop seams through the OperatorPage, and assert the Song returned by the repository has the expected background. The first test targets the service-order drop; the second targets a cue-card drop and checks all later slides.

- [ ] **Step 2: Run and observe RED**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "asset drop persists" --output-on-failure
```

Expected: compilation fails because the main-window drop handlers are not wired.

- [ ] **Step 3: Implement repository-safe mutation helpers**

`apply_asset_to_song` must:

```cpp
if (!repository_ || content.kind != domain::ContentKind::song) return;
const auto asset_record = repository_->load_asset(asset);
if (!asset_record) return;
auto song = repository_->load_song(std::get<domain::SongId>(content.id));
if (!song) return;
application::set_default_background(*song,
    domain::BackgroundRef{asset_record->id, domain::FitMode::cover});
const auto saved = repository_->save_song(*song);
if (!saved) return;
presentation_.register_song(*saved);
refresh_cues();
update_program_status();
```

`apply_asset_to_slide` must perform the same project/asset/song validation, call `application::apply_background_from_slide`, save only on success, register the saved Song, then call `refresh_cues()` and `update_program_status()`.

- [ ] **Step 4: Run focused tests**

Run the build and the new main-window filter. Expected: the direct mutation boundaries persist the requested data.

- [ ] **Step 5: Commit**

```text
git add src/ui/app/include/shekinah/ui/main_window.hpp src/ui/app/main_window.cpp tests/ui/main_window_test.cpp
git commit -m "feat: add main-window background mutation boundaries"
```

### Task 15: Wire Service Order drops to Song defaults

**Files:**
- Modify: `src/ui/app/main_window.cpp`
- Test: `tests/ui/project_session_test.cpp`

**Interfaces:**
- The constructor registers `set_service_order_asset_drop_handler` with `apply_asset_to_song`.

- [ ] **Step 1: Write the failing integration test**

Add a project-session test that imports an image and Song, appends the Song to the order, locates `service-order-list`, sends a `QDropEvent` with `make_asset_mime_data(asset->id)` at the Song row, reloads the Song from SQLite, and asserts `default_background->asset == asset->id`.

- [ ] **Step 2: Run and confirm RED**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "Song-row asset drop" --output-on-failure
```

Expected: the event is ignored or the reloaded Song has no default because the callback is not registered.

- [ ] **Step 3: Wire the callback**

In the constructor after the existing OperatorPage handlers:

```cpp
operator_page_->set_service_order_asset_drop_handler(
    [this](domain::ContentRef content, domain::AssetId asset) {
        apply_asset_to_song(std::move(content), std::move(asset));
    });
```

- [ ] **Step 4: Run focused integration tests**

Run the build and `ctest --test-dir build/windows-ucrt64-debug -R "Song-row asset drop|service-order selection" --output-on-failure`. Expected: the new drop persists the Song default and existing order behavior remains green.

- [ ] **Step 5: Commit**

```text
git add src/ui/app/main_window.cpp tests/ui/project_session_test.cpp
git commit -m "feat: persist asset drops on Song rows"
```

### Task 16: Wire Cue Card drops to Song slide ranges

**Files:**
- Modify: `src/ui/app/main_window.cpp`
- Test: `tests/ui/project_session_test.cpp`

**Interfaces:**
- The constructor registers `set_cue_asset_drop_handler` with `apply_asset_to_slide`.

- [ ] **Step 1: Write the failing integration test**

Import a three-slide Song and an image, add/select the Song, locate `cue-<second-slide-id>`, send a valid `QDropEvent` to its center, reload the Song, assert the first slide is unchanged, and assert the second and third slide backgrounds use the dropped asset.

- [ ] **Step 2: Run and confirm RED**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "cue-card asset drop" --output-on-failure
```

Expected: the drop event is ignored or the stored slides remain unchanged.

- [ ] **Step 3: Wire the callback**

In the constructor:

```cpp
operator_page_->set_cue_asset_drop_handler(
    [this](domain::CueId cue, domain::AssetId asset) {
        apply_asset_to_slide(std::move(cue), std::move(asset));
    });
```

- [ ] **Step 4: Run focused integration tests**

Run the build and `ctest --test-dir build/windows-ucrt64-debug -R "cue-card asset drop|imported image can become" --output-on-failure`. Expected: forward propagation and existing one-slide background behavior both pass.

- [ ] **Step 5: Commit**

```text
git add src/ui/app/main_window.cpp tests/ui/project_session_test.cpp
git commit -m "feat: persist asset drops on Song slides"
```

### Task 17: Refresh selected cues and live preview after drops

**Files:**
- Modify: `src/ui/app/main_window.cpp`
- Test: `tests/ui/project_session_test.cpp`

**Interfaces:**
- Successful drops refresh the selected cue cards even when the live cue is not the dropped card.
- Successful drops update the live `ProgramStatus`/Audience Output when the live cue resolves to the changed background.

- [ ] **Step 1: Write the failing regression test**

Extend the cue-card integration test by taking the second cue live before dropping the asset. After the drop, assert the cue card `cue-background-path` property resolves to the imported path and `window.audience_output()->slide_image_loaded()` is true for an image.

- [ ] **Step 2: Run and confirm RED**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "drop refreshes live" --output-on-failure
```

Expected: the stored Song may change, but the already-rendered cue or live preview remains stale.

- [ ] **Step 3: Keep refresh ordering explicit**

After a successful repository save, the handler must execute in this order:

```cpp
presentation_.register_song(*saved);
refresh_cues();
update_program_status();
```

Do not call `update_program_status()` before registering the saved Song.

- [ ] **Step 4: Run focused UI/session tests**

Run the build and:

```text
ctest --test-dir build/windows-ucrt64-debug -R "drop refreshes live|Audience Output|cue card paints a usable" --output-on-failure
```

Expected: the refreshed cue and live output assertions pass.

- [ ] **Step 5: Commit**

```text
git add src/ui/app/main_window.cpp tests/ui/project_session_test.cpp
git commit -m "fix: refresh previews after asset background drops"
```

### Task 18: Add later-drop range replacement coverage

**Files:**
- Modify: `tests/application/song_backgrounds_test.cpp`
- Modify: `tests/ui/project_session_test.cpp`

**Interfaces:**
- A second drop at a later cue replaces that cue and every following slide while leaving the earlier range intact.

- [ ] **Step 1: Write the failing regression test**

Add a pure helper test:

```cpp
TEST_CASE("a later slide drop starts a new background range") {
    Song song{SongId{"song"}, "Song", std::nullopt,
              {Slide{CueId{"one"}, "Verse", {"One"}, BackgroundRef{AssetId{"first"}}},
               Slide{CueId{"two"}, "Chorus", {"Two"}, BackgroundRef{AssetId{"first"}}},
               Slide{CueId{"three"}, "Bridge", {"Three"}, BackgroundRef{AssetId{"first"}}}}};

    REQUIRE(apply_background_from_slide(song, CueId{"two"}, BackgroundRef{AssetId{"second"}}));
    REQUIRE(song.slides[0].background->asset == AssetId{"first"});
    REQUIRE(song.slides[1].background->asset == AssetId{"second"});
    REQUIRE(song.slides[2].background->asset == AssetId{"second"});
}
```

- [ ] **Step 2: Run and observe RED**

Run the helper filter. Expected: if the implementation incorrectly changes only one slide or the entire Song, the assertions fail.

- [ ] **Step 3: Keep the range loop minimal**

The helper must begin at `std::find_if`'s returned iterator and assign only from that iterator through `song.slides.end()`; it must never touch earlier slides.

- [ ] **Step 4: Run helper and session tests**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "background range|cue-card asset drop" --output-on-failure
```

Expected: all range replacement tests pass.

- [ ] **Step 5: Commit**

```text
git add tests/application/song_backgrounds_test.cpp tests/ui/project_session_test.cpp
git commit -m "test: cover replacement background ranges"
```

### Task 19: Add project reopen persistence coverage

**Files:**
- Modify: `tests/ui/project_session_test.cpp`

**Interfaces:**
- Reopening the same project returns the Song with the assigned default and per-slide backgrounds.

- [ ] **Step 1: Write the failing test**

After the existing drop event assertions, destroy the first `ShekinahWindow`, construct a second one, call `open_project`, load the Song through the project repository boundary available in the test, and assert all stored `BackgroundRef::asset` values.

- [ ] **Step 2: Run and confirm current persistence**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "reopen.*background|project reopen.*background" --output-on-failure
```

Expected: the test passes only if the repository save path wrote every slide in the range; otherwise it fails with a missing or old asset id.

- [ ] **Step 3: Fix only persistence defects**

If the test fails, inspect the first failing repository assertion and correct the Song save/load path without changing the database schema or weakening the test.

- [ ] **Step 4: Run storage and session tests**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "reopen.*background|SQLite content repository|project session" --output-on-failure
```

Expected: all selected storage/session tests pass.

- [ ] **Step 5: Commit**

```text
git add tests/ui/project_session_test.cpp
git commit -m "test: verify dropped backgrounds survive reopen"
```

### Task 20: Add accessible drop affordances and tile polish

**Files:**
- Modify: `src/ui/widgets/library_dock/library_dock_widget.cpp`
- Modify: `src/ui/widgets/service_order/service_order_widget.cpp`
- Modify: `src/ui/widgets/cue_grid/cue_grid_widget.cpp`
- Test: `tests/ui/library_dock_test.cpp`, `tests/ui/service_order_widget_test.cpp`, `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- Asset tiles expose tooltips with path and byte size.
- Drop surfaces accept copy actions and reject unsupported MIME payloads without changing selection.
- Cue cards may set a `cue-drop-target` dynamic property while a valid asset drag enters and clear it on leave/drop.

- [ ] **Step 1: Write the failing tests**

Assert that an asset tile tooltip contains both the stored relative path and byte count, that a non-asset `QMimeData` is ignored by the Service Order list, and that a valid cue drop leaves the card's `cue-drop-target` property false after the drop.

- [ ] **Step 2: Run focused tests and observe RED**

Run:

```text
ctest --test-dir build/windows-ucrt64-debug -R "tooltip|unsupported MIME|drop-target" --output-on-failure
```

Expected: the tooltip assertion may pass already; the unsupported-MIME and dynamic-property assertions fail until the explicit boundaries are implemented.

- [ ] **Step 3: Implement the polish**

Use `setToolTip` for the existing metadata, accept only `Qt::CopyAction` in drag-enter/move/drop handlers, and update the cue-card property as follows:

```cpp
setProperty("cue-drop-target", true);
update();
```

Clear it in `dragLeaveEvent`, at the beginning of `dropEvent`, and after invalid payloads. Do not add fake labels or extra data.

- [ ] **Step 4: Run focused UI tests**

Run the build and the new filters plus the complete UI executable. Expected: all new affordance tests and all existing UI tests pass.

- [ ] **Step 5: Commit**

```text
git add src/ui/widgets/library_dock/library_dock_widget.cpp src/ui/widgets/service_order/service_order_widget.cpp src/ui/widgets/cue_grid/cue_grid_widget.cpp tests/ui/library_dock_test.cpp tests/ui/service_order_widget_test.cpp tests/ui/cue_grid_test.cpp
git commit -m "polish: clarify asset drop affordances"
```

### Task 21: Run complete CI gates and document the final state

**Files:**
- Modify: `docs/superpowers/plans/2026-11-21-asset-background-drag-plan.md` only to mark completed task checkboxes after implementation.

**Interfaces:**
- No production interface changes; this task verifies the full repository gate.

- [ ] **Step 1: Run the complete debug gate**

```text
cmake --preset windows-ucrt64-debug
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug --output-on-failure
./build/windows-ucrt64-debug/shekinah_n4000_bench.exe --json shekinah-n4000.json
```

Expected: configure/build exit 0, 100% tests passed, and the benchmark exits 0. The benchmark command must be run from PowerShell as `./build/windows-ucrt64-debug/shekinah_n4000_bench.exe --json shekinah-n4000.json`.

- [ ] **Step 2: Run the complete release gate**

```text
cmake --preset windows-ucrt64-release
cmake --build --preset windows-ucrt64-release --parallel 2
ctest --test-dir build/windows-ucrt64-release --output-on-failure
```

Expected: configure/build exit 0 and 100% release tests passed.

- [ ] **Step 3: Inspect the final diff and commit list**

```text
git diff --check
git status --short
git log --oneline --decorate -24
```

Expected: no whitespace errors, only task files changed, and at least 20 new meaningful commits after the approved spec commit.

- [ ] **Step 4: Update the plan checkboxes**

Mark completed steps in this file only after their command output has been read and verified. Do not mark a task complete based on an agent report or an assumed result.

- [ ] **Step 5: Commit documentation and cleanup**

```text
git add docs/superpowers/plans/2026-11-21-asset-background-drag-plan.md
git commit -m "docs: record asset background drag delivery"
```

## Self-review checklist

- [ ] Every approved behavior has a task: tile presentation, drag payload, Song default drop, Song slide range drop, later range replacement, Sermon rejection, persistence, and preview refresh.
- [ ] Every new production interface has a named file and a test boundary.
- [ ] No database migration is introduced.
- [ ] The plan does not require fabricated asset dates or labels.
- [ ] The plan preserves the current one-slide context-menu behavior.
- [ ] The final commands match `.github/workflows/ci.yml`.
- [ ] The commit sequence reaches at least 20 meaningful commits without empty commits.
