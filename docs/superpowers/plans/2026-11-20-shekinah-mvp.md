# Shekinah MVP Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver a Windows-first, C++20 worship-presentation MVP with reusable Songs, Sermon decks, media assets, direct live cue control, SQLite persistence, and bounded memory use.

**Architecture:** A Qt 6 Widgets shell is split using Feature-Sliced Design and dispatches commands to a Qt-free C++ domain/application core. SQLite and media implementations sit behind application ports; projects contain a SQLite database plus file-backed, hash-addressed media. The audience output is a distinct window, while the operator console stays a direct live console with a floating non-live Inspector.

**Tech Stack:** C++20, CMake 4.4+, Ninja, Qt 6 Widgets/Multimedia/Test, SQLite 3, Catch2 3, nlohmann-json, MSYS2 UCRT64 on Windows, GitHub Actions.

## Global Constraints

- Target Windows 10/11 first; keep `shekinah-core` free of Qt, Win32, and SQLite headers so a Linux adapter can be added later.
- Use C++20 and treat warnings as errors in CI: `/W4 /WX` for MSVC and `-Wall -Wextra -Wpedantic -Werror` for GCC/Clang.
- Use SQLite as the authoritative project database; media files remain outside the database in `assets/<sha256>.<extension>`.
- Preserve unlimited service-order capacity. Store references, not duplicated content or media, and virtualize long UI lists.
- Support local PNG/JPG still images and local MP4/H.264 looping backgrounds. Reject unsupported imports with a structured error.
- Default cue-grid columns: 3 at 1366×768, 4 at 1920×1080, 5 on larger desktop widths, and no more than 6; use 1–2 only when the available viewport is too narrow.
- Left-clicking a cue takes it live. Selecting a service-order item never changes program. Right-clicking a cue opens per-slide actions; Inspector is a floating non-live popup.
- Put Previous, Next, Clear, and Black directly beneath Program Status. Do not add a permanent preview surface or a Studio Mode workflow.
- Keep representative working set under 100 MB when practical and fail the stress-performance test above 300 MB.
- Treat Intel Celeron N4000 with 4 GB RAM and UHD 600 graphics as the minimum target. Its LowEndCompatibilityProfile limits audience output to 1080p30, permits one active 1080p30 MP4/H.264 background, requires a hardware-decode preflight, and falls back to still images when that profile cannot be met.
- Cap dynamic memory pools at 16 MiB asset thumbnails, 12 MiB cue artwork, 36 MiB active still textures, 48 MiB active video frames, 16 MiB import/decode staging, and 8 MiB SQLite/file-I/O buffers. At 220,000,000/250,000,000/270,000,000 bytes, ResourceGovernor respectively evicts non-live caches, cancels queued work/releases non-active buffers, and rejects new live-media loads before program state changes. The process must remain below 300,000,000 bytes.
- Every behavior change follows red-green-refactor. Never use focused, skipped, or weakened tests.
- Keep individual test lanes below 110 seconds; use deterministic fixtures and no live network input.
- Build every Song through a Reflow document: continuous UTF-8 text plus validated slide-break positions. `Ctrl+Enter` inserts a break, Backspace at the following text start merges slides, and normal Enter remains a lyric line break.
- Produce at least 85 nonempty, green, meaningful commits. Do not commit red tests, broken code, formatting-only fragments, or no-op changes merely to increase the count.

## Commit Cadence

The task commits below are delivery milestones, not the complete commit count. Within each task, preserve each independently buildable green behavior as its own commit. The minimum schedule is: Task 1 (7 commits), Task 2 (10), Task 3 (12), Task 4 (8), Task 5 (12), Task 6 (7), Task 7 (7), Task 8 (8), Task 9 (6), Task 10 (8), Task 11 (7), Task 12 (6), and Task 13 (5): **103 meaningful commits total**. Each commit must include its passing tests and must leave its affected CTest labels green.

## Repository Map

```text
CMakeLists.txt                         # target graph and test registration
CMakePresets.json                      # UCRT64 debug/release/CI commands
cmake/CompilerWarnings.cmake           # warning policy
cmake/ShekinahTesting.cmake             # Catch2 discovery and labels
src/core/domain/                        # IDs, entities, presentation state
src/core/application/                   # commands, ports, use cases
src/storage/sqlite/                     # migrations, repositories, import transactions
src/media/                              # bounded image cache and playback interfaces
src/ui/app/                             # Qt application composition root
src/ui/pages/operator/                  # operator workspace
src/ui/widgets/                         # service order, cue grid, program status, library dock
src/ui/features/                        # present cue, imports, editable Inspector actions
src/ui/entities/                        # Qt view-model adapters for core entities
tests/domain/                           # pure Catch2 tests
tests/application/                      # use-case tests using fakes
tests/storage/                          # temporary SQLite/project-folder integration tests
tests/media/                            # cache and media-adapter contract tests
tests/ui/                               # QTest interaction contracts
tests/e2e/                              # operator-to-output smoke test
tests/performance/                      # process-memory stress test
tests/fixtures/                         # checked-in deterministic text/PNG/MP4 fixtures
.github/workflows/ci.yml                # blocking Windows CI
docs/development.md                     # local build and verification commands
```

## Task 1: Reproducible build, dependency, and test harness

**Files:**
- Create: `CMakeLists.txt`, `CMakePresets.json`, `cmake/CompilerWarnings.cmake`, `cmake/ShekinahTesting.cmake`, `docs/development.md`
- Create: `src/ui/app/main.cpp`, `tests/CMakeLists.txt`, `tests/smoke/smoke_test.cpp`, `tests/support/test_project.hpp`
- Create: `.github/workflows/ci.yml`

**Interfaces:** Produces CMake targets `shekinah_app`, `shekinah_domain_tests`, `shekinah_smoke_tests`, and CTest labels `domain`, `smoke`.

- [ ] **Step 1: Write the failing smoke test.**

```cpp
#include <catch2/catch_test_macros.hpp>
TEST_CASE("build harness exposes the Shekinah application target") {
    REQUIRE(true);
}
```

- [ ] **Step 2: Verify the expected configuration failure.**

Run: `cmake --preset windows-ucrt64-debug`

Expected: configuration fails because `CMakePresets.json` and target definitions do not yet exist.

- [ ] **Step 3: Add the minimal build definition.**

```cmake
cmake_minimum_required(VERSION 3.28)
project(Shekinah VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Widgets Multimedia Test)
find_package(SQLite3 REQUIRED)
find_package(Catch2 3 REQUIRED)
add_executable(shekinah_app src/ui/app/main.cpp)
target_link_libraries(shekinah_app PRIVATE Qt6::Widgets Qt6::Multimedia)
add_subdirectory(tests)
```

`main.cpp` must construct `QApplication`, create one `QMainWindow` titled `Shekinah`, show it, and return `app.exec()`. Configure UCRT64 packages in `docs/development.md` with `mingw-w64-ucrt-x86_64-qt6-base`, `mingw-w64-ucrt-x86_64-qt6-multimedia`, `mingw-w64-ucrt-x86_64-sqlite3`, and `mingw-w64-ucrt-x86_64-catch2`. The CI workflow must use `msys2/setup-msys2`, install those packages, configure, build, and run all CTest labels without `continue-on-error`.

`tests/support/test_project.hpp` must provide `TestProject::create()` for a unique temporary project root and `fixture_path(std::string_view)` for deterministic files under `tests/fixtures`; every later integration test uses these helpers rather than user directories.

- [ ] **Step 4: Verify the harness.**

Run: `cmake --preset windows-ucrt64-debug; cmake --build --preset windows-ucrt64-debug; ctest --test-dir build/windows-ucrt64-debug --output-on-failure`

Expected: build exit code 0 and `100% tests passed`.

- [ ] **Step 5: Commit.**

```powershell
git add CMakeLists.txt CMakePresets.json cmake docs/development.md src/ui/app/main.cpp tests .github/workflows/ci.yml
git commit -m "build: add Qt C++ test harness"
```

## Task 2: Domain entities and direct-live presentation state

**Files:**
- Create: `src/core/domain/include/shekinah/domain/model.hpp`, `src/core/domain/include/shekinah/domain/presentation_engine.hpp`, `src/core/domain/presentation_engine.cpp`
- Create: `tests/domain/presentation_engine_test.cpp`, `src/core/domain/CMakeLists.txt`

**Interfaces:** Produces `SongId`, `SermonId`, `AssetId`, `ServiceOrderItemId`, `CueId`, `ContentRef`, `ProgramMode`, `ProgramState`, and `PresentationEngine`.

- [ ] **Step 1: Write failing state tests.**

```cpp
TEST_CASE("selecting content never changes the current program") {
    PresentationEngine engine;
    engine.take_live(CueRef{ContentRef::song("song-a"), CueId{"chorus-2"}});
    engine.select_content(ContentRef::song("song-b"));
    REQUIRE(engine.program().live_cue->cue.value == "chorus-2");
}
TEST_CASE("taking a cue live replaces the program cue") {
    PresentationEngine engine;
    engine.take_live(CueRef{ContentRef::song("song-a"), CueId{"bridge"}});
    REQUIRE(engine.program().mode == ProgramMode::cue);
    REQUIRE(engine.program().live_cue->cue.value == "bridge");
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R presentation_engine --output-on-failure`

Expected: compile failure because `PresentationEngine` is absent.

- [ ] **Step 3: Implement the minimal state machine.**

```cpp
enum class ProgramMode { cue, clear, black };
enum class ErrorCode { invalid_input, not_found, io_failure, storage_failure, unsupported_media, copy_failed };
struct Error { ErrorCode code; std::string message; };
template<class T> class Result {
public: static Result ok(T); static Result fail(Error); explicit operator bool() const noexcept;
  T& value(); const T& value() const; const Error& error() const;
};
using Status = Result<std::monostate>;
struct CueRef { ContentRef content; CueId cue; };
struct ProgramState { ProgramMode mode{ProgramMode::clear}; std::optional<CueRef> live_cue; };
class PresentationEngine {
public:
  void select_content(ContentRef content) { selected_content_ = std::move(content); }
  void take_live(CueRef cue) { program_ = {ProgramMode::cue, std::move(cue)}; }
  void clear() { program_ = {ProgramMode::clear, std::nullopt}; }
  void black() { program_ = {ProgramMode::black, std::nullopt}; }
  [[nodiscard]] const ProgramState& program() const noexcept { return program_; }
private: std::optional<ContentRef> selected_content_; ProgramState program_{};
};
```

- [ ] **Step 4: Verify domain tests and warnings.**

Run: `cmake --build --preset windows-ucrt64-debug; ctest --test-dir build/windows-ucrt64-debug -L domain --output-on-failure`

Expected: all domain tests pass with no compiler warnings.

- [ ] **Step 5: Commit.**

```powershell
git add src/core/domain tests/domain CMakeLists.txt tests/CMakeLists.txt
git commit -m "feat: add presentation domain state"
```

## Task 3: SQLite schema, migrations, and safe project folders

**Files:**
- Create: `src/storage/sqlite/include/shekinah/storage/project_store.hpp`, `src/storage/sqlite/migrations.cpp`, `src/storage/sqlite/project_store.cpp`
- Create: `tests/storage/project_store_test.cpp`

**Interfaces:** Consumes domain IDs. Produces `ProjectStore::create_or_open(path)`, `ProjectStore::migrate()`, and `ProjectStore::transaction(Fn)`.

- [ ] **Step 1: Write failing integration tests in a temporary directory.**

```cpp
TEST_CASE("opening a project creates a migrated SQLite database and assets directory") {
  TemporaryDirectory temp;
  auto store = ProjectStore::create_or_open(temp.path() / "Sunday");
  REQUIRE(store.has_value());
  REQUIRE(std::filesystem::exists(temp.path() / "Sunday" / "shekinah.sqlite"));
  REQUIRE(std::filesystem::is_directory(temp.path() / "Sunday" / "assets"));
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R project_store --output-on-failure`

Expected: compilation fails because `ProjectStore` is absent.

- [ ] **Step 3: Implement versioned migrations and transactions.**

```cpp
class ProjectStore {
public:
  static Result<ProjectStore> create_or_open(const std::filesystem::path& root);
  Status migrate();
  template<class Fn> Status transaction(Fn&& fn);
};
```

Migration 1 must create `schema_migrations`, `assets`, `songs`, `song_slides`, `sermon_decks`, `sermon_slides`, `service_orders`, `service_order_items`, and `slide_backgrounds` with foreign keys enabled. Store all asset paths relative to the project folder and run `PRAGMA foreign_keys = ON` on every connection.

- [ ] **Step 4: Verify migrations and rollback.**

Run: `ctest --test-dir build/windows-ucrt64-debug -L storage --output-on-failure`

Expected: fresh open, reopen, foreign-key, and transaction-rollback tests pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/storage/sqlite tests/storage CMakeLists.txt tests/CMakeLists.txt
git commit -m "feat: add SQLite project storage"
```

## Task 4: Asset import and bounded metadata-first library

**Files:**
- Create: `src/core/application/include/shekinah/application/asset_port.hpp`, `src/storage/sqlite/asset_importer.cpp`
- Create: `src/media/include/shekinah/media/image_cache.hpp`, `src/media/image_cache.cpp`
- Create: `tests/storage/asset_importer_test.cpp`, `tests/media/image_cache_test.cpp`

**Interfaces:** Produces `AssetImporter::import_file(path) -> Result<AssetRecord>`, `ImageCache::get_or_load(AssetId)`, and `ImageCache::resident_bytes()`.

- [ ] **Step 1: Write failing tests for hash deduplication and eviction.**

```cpp
TEST_CASE("same bytes imported twice produce one asset record") {
  auto first = importer.import_file(fixture("background.png"));
  auto second = importer.import_file(fixture("background-copy.png"));
  REQUIRE(first.value().id == second.value().id);
}
TEST_CASE("image cache evicts least recently used entries above its byte cap") {
  ImageCache cache{1024};
  cache.put(AssetId{"a"}, ImageBytes{700}); cache.put(AssetId{"b"}, ImageBytes{700});
  REQUIRE(cache.resident_bytes() <= 1024);
  REQUIRE_FALSE(cache.contains(AssetId{"a"}));
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R "asset_importer|image_cache" --output-on-failure`

Expected: missing importer/cache symbols fail compilation.

- [ ] **Step 3: Implement strict import policy.**

```cpp
enum class AssetKind { image, video };
struct AssetRecord { AssetId id; AssetKind kind; std::string sha256; std::filesystem::path relative_path; };
class AssetImporter { public: Result<AssetRecord> import_file(const std::filesystem::path& source); };
class ImageCache { public: explicit ImageCache(std::size_t byte_cap); void put(AssetId, ImageBytes); [[nodiscard]] std::size_t resident_bytes() const; };
```

Accept PNG/JPG images and MP4 files whose video stream reports H.264; copy verified bytes to `assets/<sha256>.<extension>`, then write metadata in one transaction. Return `unsupported_media`, `unreadable_file`, or `copy_failed` errors without leaving orphan metadata. Keep only metadata and small thumbnails in the library until an asset is rendered.

- [ ] **Step 4: Verify asset and cache tests.**

Run: `ctest --test-dir build/windows-ucrt64-debug -L "storage|media" --output-on-failure`

Expected: imports, duplicate hashes, invalid media, rollback, and byte-cap tests pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/core/application src/storage/sqlite src/media tests/storage tests/media tests/fixtures
git commit -m "feat: add project asset import and image cache"
```

## Task 5: Reusable Songs, Sermons, and unlimited service order

**Files:**
- Create: `src/core/application/include/shekinah/application/content_repository.hpp`, `src/core/application/include/shekinah/application/song_reflow.hpp`, `src/core/application/service_order_service.cpp`, `src/core/application/song_reflow.cpp`
- Create: `src/storage/sqlite/content_repository.cpp`, `src/storage/sqlite/song_text_importer.cpp`
- Create: `tests/application/service_order_service_test.cpp`, `tests/application/song_reflow_test.cpp`, `tests/storage/content_repository_test.cpp`

**Interfaces:** Produces `ContentRepository`, `ServiceOrderService::append(ContentRef)`, `SongTextImporter::parse(string_view)`, `SongReflowDocument`, and `SermonDeckImporter::import_pngs(vector<path>)`.

- [ ] **Step 1: Write failing reuse and scale tests.**

```cpp
TEST_CASE("a service order accepts repeated references without duplicating a song") {
  for (int i = 0; i != 500; ++i) service.append(ContentRef::song("song-a"));
  REQUIRE(service.items().size() == 500);
  REQUIRE(repository.song_count() == 1);
}
TEST_CASE("song text uses section headers and blank lines as slide boundaries") {
  auto song = SongTextImporter::parse("# Goodness\n[Verse 1]\nLine one\n\nLine two\n[Chorus]\nSing");
  REQUIRE(song.value().slides.size() == 3);
  REQUIRE(song.value().slides[2].section == "Chorus");
}
TEST_CASE("inserting then removing a Reflow break splits then merges a Song slide") {
  SongReflowDocument doc{"First line\nSecond line", {}};
  REQUIRE(doc.insert_break_after_utf8_offset(10)); REQUIRE(doc.to_slides().size() == 2);
  REQUIRE(doc.remove_break_before_utf8_offset(11)); REQUIRE(doc.to_slides().size() == 1);
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R "service_order|content_repository" --output-on-failure`

Expected: tests fail because content repositories and importers are absent.

- [ ] **Step 3: Implement reusable references.**

```cpp
class ContentRepository {
public:
  Result<Song> save_song(Song song); Result<SermonDeck> save_deck(SermonDeck deck);
  Result<std::vector<ServiceOrderItem>> load_order(ServiceOrderId id) const;
};
class ServiceOrderService { public: Result<ServiceOrderItem> append(ServiceOrderId, ContentRef); };
struct SlideBreak { std::size_t utf8_offset; std::string section_label; };
class SongReflowDocument {
public: Status insert_break_after_utf8_offset(std::size_t); Status remove_break_before_utf8_offset(std::size_t);
  [[nodiscard]] std::vector<SongSlide> to_slides() const;
};
```

Persist `service_order_items(content_kind, content_id, ordinal)` without a unique constraint on `(content_kind, content_id)`. Add `song_reflow_documents(song_id, utf8_text)` and `song_slide_breaks(song_id, ordinal, utf8_offset, section_label)` migrations. Validate every break sits on a UTF-8 code-point boundary, derive `song_slides` transactionally after each save, sort PNG import by normalized filename, reject duplicate ordinal input, and save only asset references. Provide archive operations that refuse deletion while references exist unless the caller explicitly removes or replaces those references.

- [ ] **Step 4: Verify application and storage tests.**

Run: `ctest --test-dir build/windows-ucrt64-debug -L "application|storage" --output-on-failure`

Expected: 500-entry reference test, Song parsing, Sermon ordering, archives, and reopen persistence pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/core/application src/storage/sqlite tests/application tests/storage tests/fixtures
git commit -m "feat: add reusable content and service orders"
```

## Task 6: Application commands, navigation, and background precedence

**Files:**
- Create: `src/core/application/include/shekinah/application/presentation_service.hpp`, `src/core/application/presentation_service.cpp`, `src/core/application/fakes.hpp`
- Create: `tests/application/presentation_service_test.cpp`

**Interfaces:** Consumes `PresentationEngine`, `ContentRepository`, and `AssetRecord`. Produces `select_service_item`, `take_live`, `previous`, `next`, `clear`, `black`, and `resolve_background`.

- [ ] **Step 1: Write failing command tests.**

```cpp
TEST_CASE("previous and next are relative to the current live cue") {
  service.take_live(item, CueId{"chorus-3"}); service.previous();
  REQUIRE(service.program().live_cue->cue.value == "bridge");
}
TEST_CASE("slide background overrides the song default") {
  auto background = service.resolve_background(song, CueId{"bridge"});
  REQUIRE(background); REQUIRE(background.value()->asset.value == "bridge-video");
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R presentation_service --output-on-failure`

Expected: command-service symbols are missing.

- [ ] **Step 3: Implement command boundary.**

```cpp
class PresentationService {
public:
  Status select_service_item(ServiceOrderItemId);
  Status take_live(ServiceOrderItemId, CueId);
  Status previous(); Status next(); Status clear(); Status black();
  Result<std::optional<BackgroundRef>> resolve_background(ContentRef, CueId) const;
};
```

`select_service_item` updates loaded content only. `take_live` resolves the requested cue and changes `ProgramState`. Navigation derives the next/previous cue from the current live content order. Background resolution is exactly `slide override -> song default -> no background`.

- [ ] **Step 4: Verify application tests.**

Run: `ctest --test-dir build/windows-ucrt64-debug -L application --output-on-failure`

Expected: direct jumps, navigation, clear/black, selection isolation, and background precedence pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/core/application tests/application
git commit -m "feat: add live presentation commands"
```

## Task 7: Audience output and media playback adapters

**Files:**
- Create: `src/media/include/shekinah/media/audience_output.hpp`, `src/media/audience_output.cpp`, `src/media/qt_video_player.cpp`
- Create: `src/ui/pages/audience/audience_window.hpp`, `src/ui/pages/audience/audience_window.cpp`
- Create: `tests/media/audience_output_test.cpp`, `tests/ui/audience_window_test.cpp`

**Interfaces:** Produces `AudienceOutput::show_fullscreen(QScreen*)`, `show_movable()`, `present(ProgramState)`, and `set_video_paused(bool)`.

- [ ] **Step 1: Write failing output-mode tests.**

```cpp
TEST_CASE("movable output uses a normal window and fullscreen output uses the selected screen") {
  AudienceOutput output{fake_renderer}; output.show_movable(); REQUIRE(output.mode() == OutputMode::movable);
  output.show_fullscreen(fake_screen); REQUIRE(output.mode() == OutputMode::fullscreen);
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R audience_output --output-on-failure`

Expected: absent output adapter fails compilation.

- [ ] **Step 3: Implement output and playback boundaries.**

```cpp
class AudienceOutput {
public:
  void show_fullscreen(QScreen*); void show_movable(); void present(const ProgramState&);
  void set_video_paused(bool); [[nodiscard]] OutputMode mode() const noexcept;
};
```

Render cue, clear, and black states distinctly. Use Qt Multimedia for one active MP4/H.264 loop; release the previous player and image texture before loading a different active background. Do not create a second permanent operator preview renderer.

- [ ] **Step 4: Verify output/media tests.**

Run: `ctest --test-dir build/windows-ucrt64-debug -L "media|ui" --output-on-failure`

Expected: output-mode, clear/black rendering, loop/pause contract, and no-preview-window tests pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/media src/ui/pages/audience tests/media tests/ui
git commit -m "feat: add audience output and media playback"
```

## Task 8: FSD operator console, responsive cue grid, and global controls

**Files:**
- Create: `src/ui/pages/operator/operator_page.*`, `src/ui/widgets/service_order/service_order_widget.*`, `src/ui/widgets/cue_grid/cue_grid_widget.*`, `src/ui/widgets/program_status/program_status_widget.*`
- Create: `src/ui/features/present_cue/present_cue_controller.*`, `tests/ui/operator_page_test.cpp`

**Interfaces:** Consumes `PresentationService`; produces `CueGridWidget::column_count_for_geometry(int,int)`, `visual_rect(CueId)`, and Qt signals `service_item_selected(ServiceOrderItemId)`, `cue_left_clicked(CueId)`, `cue_right_clicked(CueId, QPoint)`.

- [ ] **Step 1: Write failing UI contract tests.**

```cpp
TEST_CASE("cue grid follows approved automatic column policy") {
  REQUIRE(CueGridWidget::column_count_for_geometry(700, 1366) == 3);
  REQUIRE(CueGridWidget::column_count_for_geometry(1000, 1920) == 4);
  REQUIRE(CueGridWidget::column_count_for_geometry(1500, 3200) == 6);
}
TEST_CASE("left clicking a cue dispatches take_live") {
  FakePresentationService service; CueGridWidget grid{service};
  grid.set_cues({CueView{CueId{"bridge"}, "Bridge", "Your goodness"}}); grid.show();
  QTest::mouseClick(&grid, Qt::LeftButton, Qt::NoModifier, grid.visual_rect(CueId{"bridge"}).center());
  REQUIRE(service.take_live_requests().back().cue.value == "bridge");
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R operator_page --output-on-failure`

Expected: cue-grid widget and controller do not yet exist.

- [ ] **Step 3: Implement the operator boundary.**

```cpp
static int column_count_for_geometry(int viewport_width, int screen_width) {
  if (viewport_width < 320) return 1; if (viewport_width < 520) return 2;
  if (screen_width < 1600) return 3; if (screen_width < 2300) return 4;
  if (screen_width < 3200) return 5; return 6;
}
```

Use a `QSplitter` for Service Order, Cue Grid, and Program Status. Place Previous, Next, Clear, and Black inside `ProgramStatusWidget`, below its status fields. Service-order selection calls `select_service_item`; cue left-click calls `take_live`; neither introduces a preview pane. Use a model/view list for the service order and only instantiate visible cue cards.

- [ ] **Step 4: Verify UI contracts.**

Run: `ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure`

Expected: columns, direct-live click, selection isolation, and control placement tests pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/ui/pages/operator src/ui/widgets src/ui/features/present_cue tests/ui
git commit -m "feat: add direct-live operator console"
```

## Task 9: Right-click slide actions and floating Inspector

**Files:**
- Create: `src/ui/features/inspect_slide/inspector_dialog.*`, `src/ui/features/edit_slide/slide_editor_dialog.*`, `src/ui/features/manage_background/background_picker_dialog.*`
- Create: `tests/ui/inspector_dialog_test.cpp`

**Interfaces:** Consumes `cue_right_clicked`; produces `InspectorDialog::open(ContentRef, CueId)` and repository-backed save actions.

- [ ] **Step 1: Write failing Inspector lifecycle test.**

```cpp
TEST_CASE("opening and closing Inspector never changes program") {
  const auto before = fake_presentation.program(); InspectorDialog dialog{fake_presentation};
  dialog.open(ContentRef::song("song-a"), CueId{"bridge"}); dialog.close();
  REQUIRE(fake_presentation.program() == before);
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R inspector_dialog --output-on-failure`

Expected: missing dialog symbols fail compilation.

- [ ] **Step 3: Implement context actions.**

```cpp
QMenu* CueGridWidget::create_context_menu(CueId cue) {
  auto* menu = new QMenu(this); menu->addAction("Open Inspector"); menu->addAction("Edit slide");
  menu->addAction("Set background"); menu->addAction("Duplicate"); menu->addAction("Delete"); return menu;
}
```

Inspector is `Qt::Tool`, non-modal, and contains editable text/section fields plus background, fit-mode, and transition controls. Save actions call application commands in a SQLite transaction; Delete requires an explicit confirmation dialog and cannot delete a live cue without first moving Program to Clear.

- [ ] **Step 4: Verify UI tests.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R "inspector|operator_page" --output-on-failure`

Expected: right-click menu, Inspector non-live lifecycle, save, duplicate, and protected-delete tests pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/ui/features/inspect_slide src/ui/features/edit_slide src/ui/features/manage_background tests/ui
git commit -m "feat: add non-live Inspector actions"
```

## Task 10: Song Reflow Editor

**Files:**
- Create: `src/ui/features/edit_song/reflow_editor_dialog.*`, `src/ui/features/edit_song/reflow_text_edit.*`
- Create: `tests/ui/reflow_editor_dialog_test.cpp`

**Interfaces:** Consumes `SongReflowDocument`; produces `ReflowEditorDialog::load_song(SongId)`, `save_song()`, and `ReflowTextEdit::insert_slide_break()`.

- [ ] **Step 1: Write failing Reflow UI tests.**

```cpp
TEST_CASE("Ctrl Enter inserts a visible Reflow break without changing lyric text") {
  ReflowEditorDialog dialog{fake_song_service}; dialog.set_text("First line\nSecond line");
  dialog.editor()->set_cursor_utf8_offset(10); QTest::keyClick(dialog.editor(), Qt::Key_Return, Qt::ControlModifier);
  REQUIRE(dialog.reflow_document().to_slides().size() == 2);
  REQUIRE(dialog.editor()->plain_text_utf8() == "First line\nSecond line");
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R reflow_editor_dialog --output-on-failure`

Expected: the Reflow dialog and text editor do not yet exist.

- [ ] **Step 3: Implement the editor-only reflow workflow.**

```cpp
class ReflowTextEdit : public QPlainTextEdit {
public: void insert_slide_break(); void remove_adjacent_slide_break(); [[nodiscard]] std::string plain_text_utf8() const;
};
class ReflowEditorDialog : public QDialog {
public: void load_song(SongId); Status save_song(); [[nodiscard]] const SongReflowDocument& reflow_document() const;
};
```

Render slide-break markers as editor decorations, not text characters. `Ctrl+Enter` inserts a marker at the cursor; Backspace at the following text start removes it; normal Enter only changes lyric text. Show derived slide thumbnails in the dialog’s right pane and update them synchronously after edits. The dialog is an editor and never calls `PresentationService::take_live`.

- [ ] **Step 4: Verify Reflow tests.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R "song_reflow|reflow_editor_dialog" --output-on-failure`

Expected: document and UI break/merge tests pass, with no Program-state changes.

- [ ] **Step 5: Commit.**

```powershell
git add src/core/application/song_reflow.* src/core/application/include/shekinah/application/song_reflow.hpp src/ui/features/edit_song tests/application/song_reflow_test.cpp tests/ui/reflow_editor_dialog_test.cpp
git commit -m "feat: add Song Reflow editor"
```

## Task 11: Bottom Library dock and reusable-content workflow

**Files:**
- Create: `src/ui/widgets/library_dock/library_dock_widget.*`, `src/ui/features/import_asset/import_asset_controller.*`, `src/ui/features/manage_content/content_library_controller.*`
- Create: `tests/ui/library_dock_test.cpp`, `tests/e2e/worship_service_test.cpp`

**Interfaces:** Produces independent `Assets`, `Songs`, and `Sermons` tabs; `add_to_service_order(ContentRef)`; and `apply_background(AssetId, BackgroundTarget)`.

- [ ] **Step 1: Write failing dock and end-to-end tests.**

```cpp
TEST_CASE("library tabs retain assets when switching to reusable content") {
  dock.show_assets(); REQUIRE(dock.visible_asset_count() == 2);
  dock.show_songs(); dock.show_assets(); REQUIRE(dock.visible_asset_count() == 2);
}
TEST_CASE("service smoke flow imports, reuses, presents, clears, and restores") {
  WorshipServiceHarness app{TestProject::create()};
  auto song = app.import_song_text("# Goodness\n[Bridge]\nYour goodness");
  auto deck = app.import_sermon_pngs({fixture_path("sermon-01.png"), fixture_path("sermon-02.png")});
  app.import_asset(fixture_path("background.png")); app.import_asset(fixture_path("loop-h264.mp4"));
  app.add_to_order(song.value()); app.add_to_order(song.value()); app.add_to_order(deck.value());
  app.select_order_item(1); app.take_live(CueId{"bridge"}); app.clear(); app.take_live(CueId{"bridge"});
  REQUIRE(app.program().live_cue->cue.value == "bridge");
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-debug -R "library_dock|worship_service" --output-on-failure`

Expected: dock and smoke fixture APIs are missing.

- [ ] **Step 3: Implement dock behavior.**

```cpp
enum class LibraryTab { assets, songs, sermons };
class LibraryDockWidget : public QDockWidget {
public: void set_tab(LibraryTab); void refresh(); signals: void content_added(ContentRef); void asset_applied(AssetId, BackgroundTarget);
};
```

Make the dock collapsible. The Assets tab imports/searches/displays media metadata and lazily generated thumbnails. The Songs tab offers `New Song` and `Import Song Text`; both open the Reflow Editor before save. The Sermons tab offers `New Sermon Deck` and `Import Sermon Deck`; import naturally sorts selected PNG filenames, presents a reorder confirmation, then saves a reusable deck. Songs and Sermons tabs also search, edit, duplicate, archive, and add reusable records to the service order. Removing a service-order item never deletes content or assets.

- [ ] **Step 4: Verify end-to-end smoke.**

Run: `ctest --test-dir build/windows-ucrt64-debug -L e2e --output-on-failure`

Expected: project creation, import, repeated references, direct cue jump, clear/black, and output restore pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/ui/widgets/library_dock src/ui/features/import_asset src/ui/features/manage_content tests/ui tests/e2e tests/fixtures
git commit -m "feat: add reusable content library dock"
```

## Task 12: Low-end compatibility profile and resource governor

**Files:**
- Create: `src/core/application/include/shekinah/application/resource_governor.hpp`, `src/core/application/resource_governor.cpp`
- Create: `src/media/include/shekinah/media/low_end_profile.hpp`, `src/media/low_end_profile.cpp`
- Create: `tests/application/resource_governor_test.cpp`, `tests/media/low_end_profile_test.cpp`, `tools/n4000_bench/main.cpp`

**Interfaces:** Produces `LowEndCompatibilityProfile`, `ResourceGovernor::on_working_set_sample(std::size_t)`, and a machine-readable N4000 benchmark executable.

- [ ] **Step 1: Write failing governor and preflight tests.**

```cpp
TEST_CASE("resource governor purges caches before it reaches the hard limit") {
  FakeResourcePools pools; ResourceGovernor governor{pools}; governor.on_working_set_sample(220'000'000);
  REQUIRE(pools.non_live_caches_purged()); governor.on_working_set_sample(250'000'000);
  REQUIRE(pools.queued_imports_cancelled()); REQUIRE(pools.non_active_media_released());
}
TEST_CASE("N4000 profile rejects nonaccelerated video before Program changes") {
  LowEndCompatibilityProfile profile; auto result = profile.preflight(VideoInfo{1920, 1080, 30, false});
  REQUIRE_FALSE(result); REQUIRE(result.error().code == ErrorCode::unsupported_media);
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-release -R "resource_governor|low_end_profile" --output-on-failure`

Expected: the profile and governor are absent.

- [ ] **Step 3: Implement bounded runtime controls.**

```cpp
class ResourceGovernor {
public: void on_working_set_sample(std::size_t bytes); [[nodiscard]] bool may_load_live_media() const noexcept;
};
struct VideoInfo { int width; int height; int frames_per_second; bool hardware_accelerated; };
class LowEndCompatibilityProfile { public: Status preflight(const VideoInfo&) const; };
```

Use one 500 ms sampler while output is active. At 220,000,000 bytes purge non-live thumbnail and cue-art caches; at 250,000,000 bytes cancel queued imports and release non-active decode resources; at 270,000,000 bytes return a recoverable preflight error before `PresentationService::take_live` can replace Program. Enforce 1080p30 maximum and one active video. `tools/n4000_bench` writes JSON with `peak_working_set_bytes`, `cpu_time_ms`, `cpu_percent_p95`, `cue_latency_p95_ms`, `frame_time_p95_ms`, `decode_path`, and `governor_transitions`; the physical N4000 gate requires respectively less than 300,000,000 bytes, at most 70% CPU p95 across two logical processors, at most 100 ms cue latency p95, and at most 33.3 ms frame-time p95.

- [ ] **Step 4: Verify deterministic low-end tests.**

Run: `ctest --test-dir build/windows-ucrt64-release -L "application|media" --output-on-failure`

Expected: pool caps, 220M/250M/270M byte actions, 1080p30 limit, and nonaccelerated-video rejection pass.

- [ ] **Step 5: Commit.**

```powershell
git add src/core/application/resource_governor.* src/core/application/include/shekinah/application/resource_governor.hpp src/media/low_end_profile.* src/media/include/shekinah/media/low_end_profile.hpp tests/application/resource_governor_test.cpp tests/media/low_end_profile_test.cpp tools/n4000_bench
git commit -m "perf: add N4000 compatibility governor"
```

## Task 13: Memory diagnostics, stress gate, and CI completion

**Files:**
- Create: `src/media/include/shekinah/media/memory_diagnostics.hpp`, `src/media/memory_diagnostics.cpp`, `tests/performance/memory_budget_test.cpp`
- Modify: `CMakeLists.txt`, `tests/CMakeLists.txt`, `.github/workflows/ci.yml`, `docs/development.md`

**Interfaces:** Produces `MemoryDiagnostics::working_set_bytes()` and a `performance` CTest label.

- [ ] **Step 1: Write a failing stress test with fixed fixtures.**

```cpp
TEST_CASE("representative worship service stays below the hard 300 MB working-set ceiling") {
  auto result = run_representative_service(fixture_project());
  CAPTURE(result.peak_working_set_bytes, result.image_cache_bytes, result.decoded_images, result.media_buffers);
  REQUIRE(result.peak_working_set_bytes < 300'000'000ULL);
}
```

- [ ] **Step 2: Verify red.**

Run: `ctest --test-dir build/windows-ucrt64-release -L performance --output-on-failure`

Expected: compilation fails because memory diagnostics and representative fixture runner are absent.

- [ ] **Step 3: Implement deterministic measurement and limits.**

```cpp
class MemoryDiagnostics {
public: [[nodiscard]] static std::size_t working_set_bytes();
};
```

Use `GetProcessMemoryInfo` on Windows and `/proc/self/status` on Linux. The fixture must load 100 1920×1080 PNG metadata records, render bounded visible thumbnails, activate one H.264 MP4 loop, and visit 500 service-order references without preloading every slide. Emit the preferred 100 MB target as a warning in test diagnostics and fail only at or above 300 MB.

- [ ] **Step 4: Run the complete local gate and inspect output.**

Run: `cmake --preset windows-ucrt64-release; cmake --build --preset windows-ucrt64-release; ctest --test-dir build/windows-ucrt64-release --output-on-failure`

Expected: all labels pass, the performance report includes working-set/cache counters, and no test lane exceeds 110 seconds.

- [ ] **Step 5: Final CI and commit.**

```powershell
git add src/media CMakeLists.txt tests/CMakeLists.txt tests/performance tests/fixtures .github/workflows/ci.yml docs/development.md
git commit -m "test: enforce Shekinah memory budget"
```

Verify the pushed GitHub Actions run after the implementation branch is pushed. Do not mark the MVP complete until the local full gate and remote blocking workflow both report success.

## Plan Self-Review

- **Spec coverage:** Tasks 2, 5, 6, 8, 9, and 10 cover the live-console, reusable content, Inspector, responsive grid, and unlimited service-order requirements. Tasks 3 and 4 cover SQLite, project folders, atomic imports, and asset lifecycle. Tasks 7 and 11 cover audience output, media constraints, and the memory gate. Task 1 establishes reproducible local and CI checks.
- **No-placeholder review:** Each task identifies exact paths, interfaces, test behavior, commands, expected result, and commit boundaries.
- **Type consistency:** `ContentRef`, `CueId`, `ServiceOrderItemId`, `PresentationService`, `AssetId`, and `ProgramState` are introduced before downstream uses. UI features invoke `PresentationService` rather than altering `ProgramState` directly.
