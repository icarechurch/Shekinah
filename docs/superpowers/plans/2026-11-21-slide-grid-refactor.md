# Slide Grid Visual Refactor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Refactor the existing Qt cue grid into responsive custom-painted slide cards that visually match the supplied reference while preserving the surrounding operator page and live-operation behavior.

**Architecture:** Keep `CueGridWidget` as the layout and interaction owner, but replace its plain `QPushButton` implementation with a private custom-painted `CueCard` widget. Extend the UI-only `CueView` projection with preview metadata, populate that metadata in `ShekinahWindow::refresh_cues()`, and synchronize live state through a new grid setter. Domain entities, persistence, media cache behavior, and the surrounding operator layout remain unchanged.

**Tech Stack:** C++20, Qt 6 Widgets (`QWidget`, `QPainter`, `QImageReader`, `QGridLayout`, `QTest`), CMake/Ninja, Catch2, CTest.

## Global Constraints

- Preserve the existing service-order panel, Program Status panel, bottom library dock, and operator-page layout.
- Preserve the existing automatic 1–6 column policy and cue ordering.
- Preserve left-click live activation and right-click context-menu routing.
- Use checkerboard for an intentionally empty or unreadable background; never fabricate a background asset.
- Resolve Song slide background override before Song default background.
- Load scaled preview images rather than retaining full-resolution image data for every cue card.
- Do not add section badge, duration, or decorative icon semantics that are absent from the current data model.
- Preserve unrelated working-tree changes in `src/media/image_cache.cpp` and `issue 1.png`.
- Run targeted checks first, then the full debug and release CTest gates.
- Keep each commit coherent and passing where practical; this plan yields ten commits including the already-created design spec and this plan.

---

### Task 1: Add presentation metadata to `CueView`

**Files:**
- Modify: `src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp`
- Test: `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- Produces `CueView::number`, `CueView::background_path`, `CueView::background_fit`, `CueView::footer_label`, and `CueView::show_text` with defaults that keep existing three-field test construction source-compatible.

- [ ] **Step 1: Write the failing metadata assertions**

Add a test case that constructs a `CueView` with the new fields and verifies the values are preserved:

```cpp
TEST_CASE("cue view carries card preview metadata") {
    CueView cue{
        CueId{"verse-1"},
        "Verse 1",
        "You are good",
        3,
        QStringLiteral("C:/assets/verse.png"),
        FitMode::cover,
        QStringLiteral("Verse 1"),
        true,
    };

    REQUIRE(cue.number == 3);
    REQUIRE(cue.background_path == QStringLiteral("C:/assets/verse.png"));
    REQUIRE(cue.background_fit == FitMode::cover);
    REQUIRE(cue.footer_label == QStringLiteral("Verse 1"));
    REQUIRE(cue.show_text);
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`

Expected: compilation fails because the new `CueView` fields do not exist.

- [ ] **Step 3: Add the fields with safe defaults**

Extend `CueView` after the existing `summary` field:

```cpp
int number{0};
QString background_path;
domain::FitMode background_fit{domain::FitMode::cover};
QString footer_label;
bool show_text{true};
```

- [ ] **Step 4: Run the focused test to verify it passes**

Run the same build and CTest command. Expected: the UI test executable builds and the metadata test passes.

- [ ] **Step 5: Commit**

```bash
git add src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp tests/ui/cue_grid_test.cpp
git commit -m "feat: add cue card preview metadata"
```

### Task 2: Introduce the custom card shell and aspect-ratio layout

**Files:**
- Modify: `src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp`
- Modify: `src/ui/widgets/cue_grid/cue_grid_widget.cpp`
- Test: `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- Produces a private `CueCard` widget with `heightForWidth(int)`, `preview_rect()`, `set_live(bool)`, and existing cue click forwarding.
- Produces `CueGridWidget::set_live_cue(std::optional<domain::CueId>)` and preserves `visual_rect()` object-name lookup.

- [ ] **Step 1: Write the failing aspect-ratio and live-state surface tests**

Add these test expectations:

```cpp
TEST_CASE("cue card reserves a 16 by 9 preview and footer") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({CueView{CueId{"one"}, "Verse", "Line", 1}});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-one"));

    REQUIRE(card != nullptr);
    REQUIRE(card->property("cue-live").toBool() == false);
    REQUIRE(card->heightForWidth(320) >= 320 * 9 / 16);
}

TEST_CASE("cue grid marks only the matching card live") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.set_cues({
        CueView{CueId{"one"}, "Verse", "Line", 1},
        CueView{CueId{"two"}, "Chorus", "Line", 2},
    });
    grid.set_live_cue(CueId{"two"});

    REQUIRE(grid.findChild<QWidget*>(QStringLiteral("cue-one"))->property("cue-live").toBool() == false);
    REQUIRE(grid.findChild<QWidget*>(QStringLiteral("cue-two"))->property("cue-live").toBool() == true);
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`

Expected: compilation fails because `set_live_cue()` and the custom card properties do not exist.

- [ ] **Step 3: Replace `CueButton` with a `CueCard` shell**

Implement the private widget as a `QWidget` that:

```cpp
class CueGridWidget::CueCard final : public QWidget {
public:
    CueCard(const CueView& cue, CueGridWidget& owner);
    int heightForWidth(int width) const override;
    bool hasHeightForWidth() const override { return true; }
    QRect preview_rect() const;
    void set_live(bool live);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
};
```

Use `QSizePolicy::Expanding/Preferred`, object names `cue-<id>`, and an outer card height of `round(width * 9 / 16) + 32` for the preview plus footer. Add `setProperty("cue-live", false)` initially.

Replace `std::vector<CueButton*>` with `std::vector<CueCard*>`, add `std::optional<domain::CueId> live_cue_`, and implement `set_live_cue()` by storing the ID and calling `set_live()` on each card.

- [ ] **Step 4: Preserve mouse behavior and run the tests**

Forward left and right mouse presses exactly as the old button did, then run the focused UI test command. Expected: existing click dispatch and new sizing/live-state tests pass.

- [ ] **Step 5: Commit**

```bash
git add src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp src/ui/widgets/cue_grid/cue_grid_widget.cpp tests/ui/cue_grid_test.cpp
git commit -m "feat: add responsive custom cue cards"
```

### Task 3: Paint the no-background checkerboard

**Files:**
- Modify: `src/ui/widgets/cue_grid/cue_grid_widget.cpp`
- Test: `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- `CueCard::paintEvent()` paints checkerboard whenever `CueView::background_path` is empty or its preview image cannot be loaded.

- [ ] **Step 1: Add a failing checkerboard render test**

Render a no-background card to a `QImage` and assert that two checker cells have different colors:

```cpp
TEST_CASE("cue card paints checkerboard when no background is assigned") {
    ensure_qapplication();
    CueGridWidget grid;
    grid.resize(320, 240);
    grid.set_cues({CueView{CueId{"empty"}, "Verse", "You are here", 1}});
    auto* card = grid.findChild<QWidget*>(QStringLiteral("cue-empty"));
    card->resize(320, card->heightForWidth(320));
    QImage image(card->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    card->render(&image);

    REQUIRE(image.pixelColor(12, 12) != image.pixelColor(24, 12));
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`. Expected: the two sampled preview pixels are identical because the card is not yet painted.

- [ ] **Step 3: Implement checkerboard painting**

In `paintEvent()`, create a preview rectangle inset from the outer border and paint alternating 16-pixel squares using `QColor("#111111")` and `QColor("#282828")`. Keep the footer outside the checkerboard.

- [ ] **Step 4: Run the focused test to verify it passes**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`. Expected: checkerboard and all prior UI tests pass.

- [ ] **Step 5: Commit**

```bash
git add src/ui/widgets/cue_grid/cue_grid_widget.cpp tests/ui/cue_grid_test.cpp
git commit -m "feat: render empty cue checkerboards"
```

### Task 4: Add scaled background image previews

**Files:**
- Modify: `src/ui/widgets/cue_grid/cue_grid_widget.cpp`
- Test: `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- `CueCard` loads `CueView::background_path` through `QImageReader` at preview dimensions and paints it according to `CueView::background_fit`.

- [ ] **Step 1: Add a failing image-preview test**

Create a temporary 40×20 image with a red left half and blue right half, pass its path through `CueView`, render the card, and assert the preview contains non-checkerboard pixels.

- [ ] **Step 2: Run the focused test to verify it fails**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`. Expected: the rendered preview remains checkerboard.

- [ ] **Step 3: Implement scaled loading and fit modes**

Add a `QImage preview_image_` and a `QSize preview_image_size_` to `CueCard`. In `resizeEvent()` or a helper called from `paintEvent()`:

```cpp
QImageReader reader(cue_.background_path);
reader.setAutoTransform(true);
reader.setScaledSize(preview_rect().size());
preview_image_ = reader.read();
```

Paint `stretch` directly, `contain` with `Qt::KeepAspectRatio`, and `cover` with `Qt::KeepAspectRatioByExpanding`, centered in the preview rectangle. If reading fails, clear the image and let checkerboard paint.

- [ ] **Step 4: Run the focused test to verify it passes**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`. Expected: image preview, fallback checkerboard, and existing tests pass.

- [ ] **Step 5: Commit**

```bash
git add src/ui/widgets/cue_grid/cue_grid_widget.cpp tests/ui/cue_grid_test.cpp
git commit -m "feat: preview scaled cue backgrounds"
```

### Task 5: Paint lyric text and footer styling

**Files:**
- Modify: `src/ui/widgets/cue_grid/cue_grid_widget.cpp`
- Test: `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- Produces `CueGridWidget::footer_color_for_section(QString)` as a testable static helper.
- `CueCard::paintEvent()` renders lyric text, accent lines, numbered footer, and section colors.

- [ ] **Step 1: Add failing color and display tests**

Add assertions for the palette mapping:

```cpp
TEST_CASE("cue card footer colors follow section names") {
    REQUIRE(CueGridWidget::footer_color_for_section("Verse 1") == QColor("#0b80ba"));
    REQUIRE(CueGridWidget::footer_color_for_section("Chorus 1") == QColor("#cc1f58"));
    REQUIRE(CueGridWidget::footer_color_for_section("Bridge") == QColor("#8336c7"));
    REQUIRE(CueGridWidget::footer_color_for_section("Tag") == QColor("#cf3a32"));
    REQUIRE(CueGridWidget::footer_color_for_section("Intro") == QColor("#0b80ba"));
}
```

Also assert the card exposes child-free presentation properties `cue-number`, `cue-footer-label`, and `cue-section-color` so the UI contract can verify metadata without relying on pixels alone.

- [ ] **Step 2: Run the focused test to verify it fails**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`. Expected: compilation fails because the helper and card properties do not exist.

- [ ] **Step 3: Implement footer and lyric painting**

Normalize `section.trimmed().toLower()` and use prefix checks for `chorus`, `bridge`, and `tag`, with blue fallback. Paint the footer at 32 pixels high, draw the number at the left, and append `footer_label` or `section` when present. For `show_text`, draw the summary centered in the preview with bold white font, a dark text shadow, and two 44-pixel accent lines above and below the text block.

- [ ] **Step 4: Run the focused test to verify it passes**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`. Expected: palette, text metadata, checkerboard/image, and interaction tests pass.

- [ ] **Step 5: Commit**

```bash
git add src/ui/widgets/cue_grid/include/shekinah/ui/cue_grid_widget.hpp src/ui/widgets/cue_grid/cue_grid_widget.cpp tests/ui/cue_grid_test.cpp
git commit -m "feat: style cue card lyrics and footers"
```

### Task 6: Integrate live highlighting and grid refresh state

**Files:**
- Modify: `src/ui/widgets/cue_grid/cue_grid_widget.cpp`
- Modify: `src/ui/app/main_window.cpp`
- Test: `tests/ui/cue_grid_test.cpp`

**Interfaces:**
- `ShekinahWindow::update_program_status()` calls `operator_page_->cue_grid()->set_live_cue(...)` with the current program cue or `std::nullopt`.

- [ ] **Step 1: Add a failing clear-state test**

Extend the live-state test to call `grid.set_live_cue(std::nullopt)` and require both cards' `cue-live` properties to be false.

- [ ] **Step 2: Run the focused test to verify it fails**

Run the focused UI build/test command. Expected: the grid retains the previous live property because clearing is not implemented.

- [ ] **Step 3: Implement synchronization**

In `CueGridWidget::set_cues()`, rebuild cards and reapply `live_cue_`. In `set_live_cue()`, update every card's property and call `update()`. In `update_program_status()`, derive the optional cue ID only when `program.live_cue` exists and pass it to the grid before/alongside Program Status updates.

- [ ] **Step 4: Run the focused and operator UI tests**

Run: `ctest --test-dir build/windows-ucrt64-debug -L "ui|e2e" --output-on-failure`

Expected: live highlighting, click behavior, operator page integration, and end-to-end tests pass.

- [ ] **Step 5: Commit**

```bash
git add src/ui/widgets/cue_grid/cue_grid_widget.cpp src/ui/app/main_window.cpp tests/ui/cue_grid_test.cpp
git commit -m "feat: synchronize live cue highlighting"
```

### Task 7: Populate image metadata from project assets

**Files:**
- Modify: `src/ui/app/main_window.cpp`
- Test: `tests/ui/main_window_test.cpp` or `tests/e2e/operator_to_output_test.cpp`

**Interfaces:**
- `refresh_cues()` creates `CueView` values with one-based numbers and absolute asset paths.
- Song cues use `presentation_.resolve_background()` to preserve slide-override/default precedence.
- Sermon cues use each sermon slide's image asset.

- [ ] **Step 1: Add a failing integration expectation**

Create a project fixture with one imported image-backed Song slide and one sermon slide, refresh the operator cues, and assert the corresponding cue card has a non-empty `background_path`. Add a no-background Song slide and assert its path is empty.

- [ ] **Step 2: Run the focused integration test to verify it fails**

Run: `cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests && ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`

Expected: `CueView` values contain no paths because `refresh_cues()` currently fills only three fields.

- [ ] **Step 3: Implement asset-path projection**

For Songs, call `presentation_.resolve_background(selected->content, slide.id)`. If it succeeds with a `BackgroundRef`, load the asset through `repository_->load_asset()` and set `background_path` to `QString::fromStdString((store_->root() / asset->relative_path).string())`, with `background_fit` from the reference. For Sermons, load the slide image asset and set `show_text = false`, `footer_label = QStringLiteral("Slide %1")`, and the same absolute path. If any lookup fails, leave the path empty.

- [ ] **Step 4: Run the focused integration test to verify it passes**

Run the focused UI test command. Expected: Song override/default precedence and sermon-image metadata pass without changing persistence or audience output behavior.

- [ ] **Step 5: Commit**

```bash
git add src/ui/app/main_window.cpp tests/ui/main_window_test.cpp tests/e2e/operator_to_output_test.cpp
git commit -m "feat: project cue backgrounds into the grid"
```

### Task 8: Add final UI regression coverage

**Files:**
- Modify: `tests/ui/cue_grid_test.cpp`
- Modify: `tests/ui/operator_page_test.cpp`

**Interfaces:**
- Locks the approved UI contract without changing production interfaces.

- [ ] **Step 1: Add regression cases**

Cover all of the following with deterministic Qt tests:

```cpp
// 1. Existing 1–6 column policy remains unchanged.
// 2. Footer label includes the one-based number.
// 3. Empty background paints checkerboard.
// 4. Unreadable background paints checkerboard.
// 5. A valid image produces non-checkerboard preview pixels.
// 6. Left click still calls TakeLiveHandler exactly once.
// 7. Right click still calls ContextHandler with the cue ID.
// 8. Live state clears after set_live_cue(std::nullopt).
// 9. Empty state still exposes authoring actions.
```

- [ ] **Step 2: Run the UI and E2E lanes**

Run: `ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`

Then run: `ctest --test-dir build/windows-ucrt64-debug -L e2e --output-on-failure`

Expected: both lanes pass with no skipped or focused tests.

- [ ] **Step 3: Commit**

```bash
git add tests/ui/cue_grid_test.cpp tests/ui/operator_page_test.cpp
git commit -m "test: lock slide grid visual behavior"
```

### Task 9: Run the complete debug quality gate

**Files:**
- Modify: none unless a verification failure identifies a root-cause fix.

**Interfaces:**
- Validates the completed feature against the documented debug CI lane.

- [ ] **Step 1: Configure and build debug**

Run: `cmake --preset windows-ucrt64-debug`

Then: `cmake --build --preset windows-ucrt64-debug --parallel 2`

- [ ] **Step 2: Run the complete debug test suite**

Run: `ctest --test-dir build/windows-ucrt64-debug --output-on-failure`

Expected: all registered tests pass, including UI, E2E, and performance-labelled tests.

- [ ] **Step 3: Commit only if a root-cause verification fix was required**

If a real defect is found, add a regression test and its minimal fix in one coherent commit:

```bash
git add <specific-test-files> <specific-source-files>
git commit -m "fix: resolve slide grid verification failure"
```

Do not commit generated build output, benchmark JSON, or unrelated working-tree files.

### Task 10: Run the complete release quality gate and inspect final state

**Files:**
- Modify: none unless a release-only root-cause fix is required.

- [ ] **Step 1: Configure and build release**

Run: `cmake --preset windows-ucrt64-release`

Then: `cmake --build --preset windows-ucrt64-release --parallel 2`

- [ ] **Step 2: Run the complete release test suite**

Run: `ctest --test-dir build/windows-ucrt64-release --output-on-failure`

Expected: all registered release tests pass.

- [ ] **Step 3: Review the final diff and status**

Run: `git diff --check`

Run: `git status --short --branch`

Confirm that only the design spec, implementation plan, intended source/test files, and the pre-existing `src/media/image_cache.cpp`/`issue 1.png` changes are present.

- [ ] **Step 4: Commit any release-only root-cause fix, if needed**

Use a focused test-plus-fix commit and rerun the affected debug and release lanes. Do not create a no-op or verification-only commit.
