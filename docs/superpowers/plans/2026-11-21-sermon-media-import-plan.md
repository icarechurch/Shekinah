# Sermon Media Import Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Allow Sermon decks to import the media formats already supported by Shekinah assets, including GIF images and MP4 videos, while preserving the existing live-output routing and resource limits.

**Architecture:** Replace the PNG-specific `SermonDeckImporter::import_pngs()` contract with `import_files()`. The importer will keep filename ordering and duplicate detection, then delegate supported-media classification and copying to `AssetImporter::import_file()`. The UI will use the same media filter as general asset import; persisted sermon slides and live image/video routing remain unchanged.

**Tech Stack:** C++20, Qt 6 Widgets/Multimedia, CMake 3.28+, Ninja, Catch2 3, SQLite, OpenSSL SHA-256 asset storage.

## Global Constraints

- Treat GitHub Actions and the repository's documented CI workflow as blocking quality gates.
- Keep all existing test coverage; do not use focused tests, skipped tests, weakened assertions, or disabled checks to obtain green results.
- Run targeted checks first, then the full relevant local gate before completion.
- Keep tests deterministic across machines, time zones, locales, calendars, random seeds, and clean checkouts.
- Preserve unrelated user changes in dirty worktrees and stage only files belonging to this task.
- Use reversible, minimal changes; do not rewrite history, force-push, or alter production behavior merely to accommodate CI.
- Supported sermon media is exactly PNG, JPG, JPEG, WEBP, BMP, GIF, TIFF, and MP4, case-insensitively.
- GIFs use the current decoded-image path; no new GIF animation controls are added.
- MP4s use the existing looping `QMediaPlayer` path; no pause/play UI is added in this change.
- No database migration is required; `SermonSlide` continues to store an `AssetId`.

---

### Task 1: Generalize the sermon importer contract and storage coverage

**Files:**
- Modify: `tests/storage/sermon_deck_importer_test.cpp`
- Modify: `src/storage/sqlite/include/shekinah/storage/sermon_deck_importer.hpp`
- Modify: `src/storage/sqlite/sermon_deck_importer.cpp`

**Interfaces:**
- Consumes: `AssetImporter::import_file(const std::filesystem::path&)`, which already classifies PNG/JPG/JPEG/WEBP/BMP/GIF/TIFF as `AssetKind::image` and MP4 as `AssetKind::video`.
- Produces: `SermonDeckImporter::import_files(domain::SermonId, std::string, std::vector<std::filesystem::path>, bool)` with the same return type and ordering semantics as the current PNG-specific method.

- [ ] **Step 1: Write the failing storage tests**

In `tests/storage/sermon_deck_importer_test.cpp`:

1. Rename each existing call from `import_pngs(...)` to `import_files(...)` and rename the PNG-specific test descriptions to “sermon files” or “supported media” wording.
2. Add a mixed-media test after the existing natural-order test:

```cpp
TEST_CASE("sermon deck importer accepts mixed supported media") {
    auto project = TestProject::create();
    auto store = ProjectStore::create_or_open(project.root() / "Sunday");
    REQUIRE(store);

    const auto jpg = project.root() / "Slide 1.jpg";
    const auto gif = project.root() / "Slide 2.GIF";
    const auto mp4 = project.root() / "Slide 3.MP4";
    std::ofstream(jpg, std::ios::binary) << "jpg-bytes";
    std::ofstream(gif, std::ios::binary) << "gif-bytes";
    std::ofstream(mp4, std::ios::binary) << "mp4-bytes";

    SermonDeckImporter importer{*store};
    const auto deck = importer.import_files(
        SermonId{"sermon-media"}, "Media Sermon", {jpg, gif, mp4}, true);

    REQUIRE(deck);
    REQUIRE(deck->slides.size() == 3);

    SqliteContentRepository repository{*store};
    const auto first = repository.load_asset(deck->slides[0].image);
    const auto second = repository.load_asset(deck->slides[1].image);
    const auto third = repository.load_asset(deck->slides[2].image);
    REQUIRE(first);
    REQUIRE(second);
    REQUIRE(third);
    REQUIRE(first->kind == AssetKind::image);
    REQUIRE(second->kind == AssetKind::image);
    REQUIRE(third->kind == AssetKind::video);
}
```

3. Keep the duplicate-name assertion and change its test title/error wording from “non-PNG files” to “unsupported media”. Keep the existing `.jpg` rejection input only if the test is changed to `.txt`; `.jpg` must now be accepted. Use this replacement for the unsupported-extension portion:

```cpp
    const auto unsupported = project.root() / "Slide 2.txt";
    std::ofstream(unsupported, std::ios::binary) << "text";

    SermonDeckImporter importer{*store};
    const auto extension_result = importer.import_files(
        SermonId{"sermon-b"}, "Sunday Sermon", {unsupported});
    REQUIRE_FALSE(extension_result);
    REQUIRE(extension_result.error().code == ErrorCode::unsupported_media);
```

- [ ] **Step 2: Run the storage tests and verify the new contract fails**

Run:

```powershell
cmake --build --preset windows-ucrt64-debug --target shekinah_storage_tests
```

Expected: compilation fails because `SermonDeckImporter` does not yet define `import_files`; no production implementation has been changed yet.

- [ ] **Step 3: Replace the PNG-specific public method**

In `src/storage/sqlite/include/shekinah/storage/sermon_deck_importer.hpp`, replace:

```cpp
    domain::Result<domain::SermonDeck> import_pngs(
        domain::SermonId id,
        std::string title,
        std::vector<std::filesystem::path> paths,
        bool preserve_order = false);
```

with:

```cpp
    domain::Result<domain::SermonDeck> import_files(
        domain::SermonId id,
        std::string title,
        std::vector<std::filesystem::path> paths,
        bool preserve_order = false);
```

In `src/storage/sqlite/sermon_deck_importer.cpp`, rename the definition to `import_files` and update the invalid-input error from:

```cpp
{domain::ErrorCode::invalid_input, "A Sermon deck needs an id, title, and at least one PNG"}
```

to:

```cpp
{domain::ErrorCode::invalid_input, "A Sermon deck needs an id, title, and at least one media file"}
```

Remove the PNG-only validation block:

```cpp
        if (path.extension().string().empty() || normalized_name.ends_with(".png") == false) {
            return domain::Result<domain::SermonDeck>::fail(
                {domain::ErrorCode::unsupported_media, "Sermon decks accept PNG slides only"});
        }
```

Keep `normalized_name` for natural ordering and duplicate detection. Leave the later `asset_importer.import_file(candidates[index].path)` call as the sole supported-extension/classification check; its existing `unsupported_media` result and supported-media message are the shared source of truth.

- [ ] **Step 4: Run the storage lane and confirm it passes**

Run:

```powershell
cmake --build --preset windows-ucrt64-debug --target shekinah_storage_tests
ctest --test-dir build/windows-ucrt64-debug -L storage --output-on-failure
```

Expected: the storage target builds and every storage test passes, including mixed GIF/MP4 acceptance, natural ordering, operator-defined ordering, duplicate detection, persistence, and unsupported-extension rejection.

- [ ] **Step 5: Commit the storage change**

```powershell
git add tests/storage/sermon_deck_importer_test.cpp src/storage/sqlite/include/shekinah/storage/sermon_deck_importer.hpp src/storage/sqlite/sermon_deck_importer.cpp
git commit -m "feat: accept media files in sermon importer"
```

---

### Task 2: Generalize sermon UI wording, file filters, and application wiring

**Files:**
- Modify: `src/ui/features/edit_sermon/sermon_editor_dialog.cpp`
- Modify: `tests/ui/sermon_editor_dialog_test.cpp`
- Modify: `src/ui/app/main_window.cpp`
- Modify: `README.md`

**Interfaces:**
- Consumes: `SermonDeckImporter::import_files(...)` from Task 1.
- Produces: media-neutral sermon creation/import/edit UI while retaining the existing `import_sermon_files(...)` window method and live image/video routing.

- [ ] **Step 1: Write the failing UI regression assertions**

In `tests/ui/sermon_editor_dialog_test.cpp`:

1. Add `#include <QLabel>`.
2. Change the test paths to prove the editor preserves non-PNG media paths:

```cpp
    dialog.set_files({"C:/slides/Slide 1.jpg", "C:/slides/Clip 2.MP4"});
```

3. Add these assertions after `dialog.file_count()`:

```cpp
    auto* description = dialog.findChild<QLabel*>(QStringLiteral("sermon-description"));
    REQUIRE(description != nullptr);
    REQUIRE(description->text().contains(QStringLiteral("media"), Qt::CaseInsensitive));

    auto* add_button = dialog.findChild<QPushButton*>(QStringLiteral("sermon-add-files"));
    REQUIRE(add_button != nullptr);
    REQUIRE(add_button->text() == QStringLiteral("Add Media"));
```

4. Update the expected filenames after moving the second row to:

```cpp
    REQUIRE(files[0].filename().string() == "Clip 2.MP4");
    REQUIRE(files[1].filename().string() == "Slide 1.jpg");
```

- [ ] **Step 2: Run the UI lane and verify the new labels fail**

Run:

```powershell
cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests
ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure
```

Expected: the editor test builds but fails because the current description has no `sermon-description` object name and the button still says `Add PNGs`.

- [ ] **Step 3: Update the sermon editor text and media filter**

In `src/ui/features/edit_sermon/sermon_editor_dialog.cpp`, update the description construction to:

```cpp
    auto* description = new QLabel{
        QStringLiteral("Add media files, then arrange them in the order they should appear."), this};
    description->setObjectName(QStringLiteral("sermon-description"));
```

Update the add button registration to:

```cpp
    add_button(QStringLiteral("Add Media"), QStringLiteral("sermon-add-files"), [this] { add_files(); });
```

Update `add_files()` to use the shared media filter:

```cpp
void SermonEditorDialog::add_files() {
    const auto paths = QFileDialog::getOpenFileNames(
        this, QStringLiteral("Add Sermon Media"), {},
        QStringLiteral(
            "Media (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.mp4);;All files (*)"));
```

Leave duplicate-path detection, item ordering, and arbitrary `set_files()`/`files()` path storage unchanged.

- [ ] **Step 4: Update the main import dialog and storage call site**

In `src/ui/app/main_window.cpp`, replace the sermon import file filter:

```cpp
    const auto paths = QFileDialog::getOpenFileNames(
        this, QStringLiteral("Import Sermon Deck"), {},
        QStringLiteral(
            "Media (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.tif *.tiff *.mp4);;All files (*)"));
```

In `ShekinahWindow::import_sermon_files`, replace:

```cpp
    return storage::SermonDeckImporter{*store_}.import_pngs(
```

with:

```cpp
    return storage::SermonDeckImporter{*store_}.import_files(
```

Do not change `import_asset()`; it already has the same media filter and should remain the reference UI wording.

- [ ] **Step 5: Update the user-facing README feature wording**

In `README.md`, replace:

```markdown
- **Sermon decks:** Import ordered PNG slides as Sermon decks and edit their slide content.
```

with:

```markdown
- **Sermon decks:** Import ordered image or video media as Sermon decks and edit their slide content.
```

- [ ] **Step 6: Run the UI lane and confirm it passes**

Run:

```powershell
cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests
ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure
```

Expected: the UI target builds and all UI tests pass, including the media-neutral sermon editor test.

- [ ] **Step 7: Commit the UI and application change**

```powershell
git add src/ui/features/edit_sermon/sermon_editor_dialog.cpp tests/ui/sermon_editor_dialog_test.cpp src/ui/app/main_window.cpp README.md
git commit -m "feat: allow media files in sermon editor"
```

---

### Task 3: Run the complete repository CI gate

**Files:**
- Verify only: `CMakeLists.txt`, `CMakePresets.json`, `.github/workflows/ci.yml`, `docs/development.md`
- No source changes are expected in this task.

**Interfaces:**
- Consumes: the two committed implementation changes from Tasks 1 and 2.
- Produces: fresh local verification for the debug and release CI lanes, plus a clear record of any unavailable environment checks.

- [ ] **Step 1: Inspect the final diff and working tree**

Run:

```powershell
git status --short
git diff HEAD~2..HEAD --check
git diff --stat HEAD~2..HEAD
rg -n "import_pngs|PNG slides|Add PNGs|at least one PNG|non-PNG" src tests README.md docs
```

Expected: only the approved design/implementation files are changed; `git diff --check` produces no output; the final search finds no active PNG-specific sermon API or UI wording. Historical design/plan text may still mention the original restriction.

- [ ] **Step 2: Run the complete debug gate**

Run:

```powershell
cmake --preset windows-ucrt64-debug
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug --output-on-failure
```

Expected: configure succeeds, the full debug build succeeds, and every CTest test passes without filters.

- [ ] **Step 3: Run the N4000 benchmark lane**

Run:

```powershell
ctest --test-dir build/windows-ucrt64-debug -L e2e --output-on-failure
ctest --test-dir build/windows-ucrt64-debug -L performance --output-on-failure
.\build\windows-ucrt64-debug\shekinah_n4000_bench.exe --json shekinah-n4000.json
```

Expected: both labeled lanes pass and the benchmark writes `shekinah-n4000.json` with the existing resource-governor metrics. Do not weaken or skip this lane if the media changes leave it unchanged.

- [ ] **Step 4: Run the complete release gate**

Run:

```powershell
cmake --preset windows-ucrt64-release
cmake --build --preset windows-ucrt64-release --parallel 2
ctest --test-dir build/windows-ucrt64-release --output-on-failure
```

Expected: configure succeeds, the full release build succeeds, and every release CTest test passes without filters.

- [ ] **Step 5: Report verification exactly**

Record the actual configure, build, test, E2E, performance, and benchmark results in the final handoff. If MSYS2/UCRT64 dependencies or Qt multimedia are unavailable locally, state the exact commands that could not run and mark the corresponding CI gate unverified; do not claim completion based only on targeted tests.

---

## Self-review checklist

- Spec coverage: mixed image/video import, case-insensitive extensions, shared asset classification, media-neutral UI, existing live routing, unchanged resource limits, unsupported-extension errors, duplicate detection, no schema change, tests, and full debug/release gates are all covered by Tasks 1–3.
- Placeholder scan: no `TBD`, `TODO`, “implement later”, or unspecified validation steps are used.
- Type consistency: `import_files(...)` is the only renamed importer method in the plan; all storage tests and the main-window call site use that exact signature.
- Scope: no pause/play UI, GIF animation rewrite, MIME sniffing, database migration, or unrelated refactor is planned.
