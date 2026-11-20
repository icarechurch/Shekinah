# Shekinah MVP Design

**Date:** 2026-11-20

**Status:** Approved for implementation planning

## Goal

Build a lightweight C++ desktop MVP for Shekinah, a ProPresenter alternative for church worship services. The MVP must support songs, sermon decks imported as PNGs, local image/video backgrounds, reusable content, live cue selection, and audience output while remaining usable on Celeron-class Windows PCs.

## Product scope

### In scope

- Windows-first desktop application with a portable core suitable for later Linux support.
- Service-order workflow containing any number of songs, sermon decks, and future content types.
- Built-in song editor and structured UTF-8 text import.
- Sermon-deck import from ordered PNG files.
- Reusable Songs and Sermons content library.
- Project-scoped Asset Library containing imported images and videos.
- Song-level default backgrounds and optional per-slide background overrides.
- Still-image backgrounds and looping local MP4/H.264 video backgrounds.
- Full-screen audience output on a selected monitor.
- Movable output window for single-monitor testing or environments without a second monitor.
- Operator live console with a responsive cue grid.
- Direct left-click cue activation for fast jumps such as Chorus 3 to Bridge.
- Right-click per-slide menu with Inspector, Edit, Set Background, Duplicate, and Delete actions.
- Optional floating Inspector popup for non-live inspection and editing.
- Previous, Next, Clear, and Black controls grouped directly below Program Status.
- Keyboard shortcuts for core live actions.
- SQLite project persistence with file-backed media.

### Explicitly out of scope for the MVP

- MIDI, OSC, remote control, stage-display output, NDI, streaming, and multi-operator collaboration.
- PowerPoint, Keynote, Canva API, or proprietary presentation-file imports.
- Audio playback or audio mixing.
- Live camera capture.
- Complex transition timelines, compositing graphs, or shader authoring.
- Network synchronization between computers.

## User workflows

### Service preparation

1. Open or create a project folder.
2. Import or create reusable Songs and Sermons.
3. Import images and MP4/H.264 videos into the Asset Library.
4. Add any number of reusable content records to the service order.
5. Reorder or remove service-order entries without deleting the referenced reusable content or assets.
6. Select a song or sermon in the service order to load its cues into the cue grid.

### Live operation

- Selecting a service-order item changes the loaded cue grid and selected content only; it does not change the audience program.
- Left-clicking a cue immediately takes that exact cue live.
- The current live cue is clearly marked in the grid and Program Status.
- Previous and Next navigate relative to the current live cue.
- Clear and Black are explicit program states.
- The operator console has no permanent preview surface and no Studio Mode workflow.
- Program Status shows the audience output monitor, full-screen/movable-output state, active cue, and active background.

### Inspector Mode

- Right-clicking a cue opens a context menu.
- Choosing Inspector opens a floating non-live popup.
- The popup can inspect and edit lyric text, section label, slide background, fit mode, and transition settings supported by the MVP.
- Closing the popup never changes the audience output.
- Left-clicking in the cue grid remains the live action, regardless of whether an Inspector popup is open.

## Operator UI layout

The operator workspace follows Feature-Sliced Design where it adds clarity to the Qt UI:

- **Service Order panel:** ordered list of reusable Songs, Sermons, and future content types.
- **Cue Grid:** all cues for the selected service-order item, using automatic 1–6 column layout.
- **Program Status panel:** current live cue, output monitor, output mode, and active background.
- **Global controls:** Previous, Next, Clear, and Black directly beneath Program Status.
- **Bottom Library dock:** three independent tabs: Assets, Songs, and Sermons.
- **Inspector:** temporary floating popup invoked from a cue’s right-click menu.

### Responsive cue-grid policy

Column count is automatic and recalculated when the operator window changes size:

- 1–2 columns when the viewport cannot preserve the minimum readable card width.
- 3 columns for 1366×768-class displays.
- 4 columns for the default 1920×1080 display class.
- 5 columns for larger desktop displays.
- 6 columns maximum for ultra-wide layouts.

Cue order and the current live cue remain unchanged during resizing. The UI must use lazy or virtualized item presentation where necessary; it must not allocate full-resolution render data for every cue in a large service.

## Library and persistence model

Each project is a portable folder:

```text
MyService/
  shekinah.sqlite
  assets/
    <content-hash>.png
    <content-hash>.mp4
```

SQLite is the authoritative project database. Media is stored as files; SQLite stores stable IDs, content hashes, paths relative to the project, media type, dimensions, duration, and other metadata.

### SQLite concepts

- `schema_migrations`
- `projects`
- `service_orders`
- `service_order_items`
- `songs`
- `song_slides`
- `sermon_decks`
- `sermon_slides`
- `assets`
- `slide_backgrounds`
- `settings`

The service order has no application-defined maximum item count. Its rows reference reusable content IDs and preserve explicit ordering. The UI may virtualize a long order, but it must not impose a small fixed limit.

Content and assets have separate lifecycles:

- deleting a service-order item removes only the reference;
- deleting or archiving a Song or Sermon does not automatically delete its referenced media;
- deleting an asset is an explicit action that checks references before removal;
- importing the same file twice reuses the existing content-hash asset record.

### Song format

The canonical editable Song record is stored in SQLite. The text importer accepts UTF-8 input with:

- title metadata;
- section markers such as `[Verse 1]` and `[Chorus]`;
- blank-line slide separation;
- deterministic slide ordering and IDs.

JSON import/export is an interchange and backup format, not the authoritative project store.

### Sermon import

PNG files are validated, ordered deterministically, copied or deduplicated into the project asset store, and recorded as ordered Sermon slides inside a single SQLite transaction.

### Atomic import behavior

1. Validate all source files.
2. Copy or deduplicate media into the project asset store.
3. Write metadata inside a SQLite transaction.
4. Roll back database changes if any metadata or asset operation fails.
5. Return structured errors suitable for display in the UI.

## Architecture

The implementation uses C++20, CMake, Qt 6 Widgets, Qt Multimedia, and SQLite. The application is divided into explicit boundaries:

- `shekinah-core/domain` — pure business entities and presentation state transitions with no Qt or SQLite dependency.
- `shekinah-core/application` — use cases, command handlers, ports, result/error types, and policies.
- `shekinah-storage` — SQLite repositories, migrations, project-folder management, asset import, and JSON interchange.
- `shekinah-media` — image cache, video playback abstraction, background selection, and platform media adapters.
- `shekinah-ui` — Qt adapter and Feature-Sliced UI organization.
- `shekinah-tests` — unit, contract, integration, UI, smoke, and performance test targets.

The core never depends on Qt. SQLite and media implementations are injected through ports. The UI dispatches application commands and observes presentation state; it does not mutate domain objects directly.

### Feature-Sliced UI mapping

- `app` — startup, dependency wiring, global shortcuts, settings, and theme.
- `pages` — operator workspace, audience output, and Inspector popup.
- `widgets` — service order, cue grid, Program Status, library dock, and context menu.
- `features` — import sermon deck, edit Song, present cue, control playback, manage assets, and manage reusable content.
- `entities` — Song, Slide, Sermon, Asset, Service Order, and Presentation State.
- `shared` — Qt-independent IDs, result types, filesystem ports, formatting, and test helpers.

## Memory and performance design

The memory budget is a hard architectural requirement:

- preferred representative-service working set: under 100 MB;
- hard stress-test ceiling: 300 MB;
- the process must fail the performance gate if the stress scenario exceeds 300 MB.

The implementation will enforce bounded resource use:

- the service order stores references rather than duplicating content;
- only the selected content is expanded into the cue grid;
- image thumbnails and decoded images use a byte-bounded cache with eviction;
- full-resolution media is decoded on demand;
- only the active video and bounded playback buffers remain live;
- the Asset Library uses metadata-first loading and lazy thumbnails;
- large cue grids and service orders use lazy/virtualized item models;
- memory diagnostics report peak working set, cache size, decoded image count, and media-buffer count.

## Test architecture

The test suite mirrors the boundaries:

- **Domain unit tests:** slide navigation, direct cue jumps, background precedence, playlist ordering, clear/black states, and invalid transitions.
- **Application contract tests:** use cases against fake repositories, asset stores, presentation outputs, and clocks.
- **SQLite integration tests:** migrations, foreign keys, persistence/reopen, unbounded service-order references, duplicate hashes, and rollback behavior.
- **Importer tests:** valid/invalid song text, UTF-8, section markers, PNG deck ordering, malformed media, and deterministic IDs.
- **Media adapter tests:** still images, supported MP4/H.264 validation, loop state, pause/resume, crop/fit policy, and fallback behavior.
- **UI contract tests:** service-order selection, left-click live behavior, right-click menu actions, Inspector popup lifecycle, responsive column policy, and unified controls.
- **End-to-end smoke test:** create a project, import a Song and PNG Sermon, add multiple entries to the service order, load cues, jump directly between cues, clear, black out, and restore output.
- **Memory test:** exercise a representative service with many 1080p PNGs, multiple reusable content records, and a looping MP4 while measuring process working set.

All behavior changes follow TDD: write a failing test, verify the expected failure, implement the smallest behavior, then run targeted and full gates. Focused tests, skipped tests, weakened assertions, and disabled CI lanes are prohibited.

## Verification and CI gates

The repository will use CMake build presets or equivalent documented commands for:

- configure;
- compile with warnings enabled;
- unit and integration tests through CTest;
- UI/headless smoke tests where supported;
- memory/performance test;
- static analysis or formatting checks where configured.

GitHub Actions will reproduce the documented Windows build/test flow and retain useful logs and failure diagnostics. Test lanes will remain blocking. Each test lane will be bounded below the two-minute execution requirement, with deterministic fixtures and no live network or time-dependent data.

## Acceptance criteria

The MVP is ready for implementation handoff when the following are true:

1. A user can create/open a SQLite-backed project folder.
2. A user can create or import a Song and edit its slides.
3. A user can import an ordered PNG Sermon deck.
4. A user can import images and MP4/H.264 videos into the Asset Library.
5. A user can reuse Songs and Sermons and add any number of them to a service order.
6. A selected service-order item loads its cue grid without changing program.
7. Left-clicking a cue takes it live immediately.
8. Right-clicking opens per-slide actions and a floating non-live Inspector.
9. Program Status owns the Previous, Next, Clear, and Black controls.
10. The cue grid automatically adapts from 1 through 6 columns and defaults to 3 at 1366×768 and 4 at 1920×1080.
11. Audience output works as selected-monitor full-screen or movable output window.
12. Asset and content references remain separate, reusable, and safe from accidental deletion.
13. The representative memory test stays below 300 MB and reports the preferred 100 MB target.
14. All required local and CI checks pass with fresh verification output.
