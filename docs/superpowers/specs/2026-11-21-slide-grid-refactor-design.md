# Slide Grid Visual Refactor Design

**Date:** 2026-11-21

**Status:** Approved for implementation planning

## Goal

Refactor the operator page's slide grid so its cards closely match the supplied Way Maker reference while preserving the existing service-order panel, Program Status panel, bottom library dock, responsive column policy, and live-operation behavior.

## Scope

### In scope

- Replace the plain text-only cue buttons with lightweight custom-painted cue cards.
- Preserve the existing automatic 1–6 column policy.
- Render a 16:9 preview area with a fixed footer band.
- Render an existing slide background image when a usable asset path is available.
- Render a dark checkerboard when no background is assigned or the background file cannot be loaded.
- Render centered white lyric text with small horizontal accent lines.
- Render numbered footer bands with section-aware colors:
  - blue for Verse and the default fallback;
  - magenta for Chorus;
  - purple for Bridge;
  - red for Tag.
- Highlight the live cue with a bright blue outline and keep hover feedback subtle.
- Preserve left-click live activation and right-click context-menu routing.
- Keep background resolution consistent with presentation behavior: slide override first, then Song default background.
- Use scaled preview images so the cue grid does not retain full-resolution image data for every card.

### Out of scope

- Reshaping the surrounding operator page.
- Adding or changing the domain model or SQLite schema.
- Adding section badge semantics, duration overlays, or decorative icons from the reference. These values are not represented by the current model.
- Adding a new asset cache or changing the existing media cache behavior.
- Changing live presentation, navigation, inspector, or context-menu semantics.

## Visual design

The cue-grid surface uses a dark charcoal background with compact margins and gaps. Each card is a `CueCard` widget placed in the existing `QGridLayout`.

The card contains:

1. A preview region sized at a 16:9 aspect ratio.
2. A footer band with a section-dependent color.
3. A left-aligned slide number in the footer and a section label when present.

For song slides, lyric lines are joined for display and centered in the preview region. Text is white, bold, and rendered with a small dark shadow/outline so it remains readable over checkerboard or image backgrounds. Two thin horizontal accent lines frame the lyric block.

For sermon slides, the imported slide image fills the preview region and the footer identifies the slide number. A missing sermon image falls back to checkerboard rather than showing a storage path.

The checkerboard is two dark charcoal tones and is painted only in the preview region. It represents the intentional absence of a background, not a generated image asset.

The live-card outline is a bright blue two-pixel border. It is visual state only and does not alter the program state. A cue that is not in the selected service-order item has no matching card and therefore receives no grid highlight.

## Data flow and interfaces

`CueGridWidget` remains responsible for:

- responsive column count;
- card layout and rebuilds;
- forwarding left-clicks to `TakeLiveHandler`;
- forwarding right-clicks to `ContextHandler`.

`CueView` becomes presentation-only metadata. It continues to hold the cue ID, section, and summary, and adds:

- one-based display number;
- optional resolved background file path;
- background fit mode;
- a flag or derived value for sermon/image preview content as needed by the card renderer.

The grid exposes a live-cue update method accepting an optional `domain::CueId`. `ShekinahWindow::update_program_status()` calls it after program state changes. The existing `PresentationService` remains the source of truth for live state.

`ShekinahWindow::refresh_cues()` constructs background metadata without mutating domain objects:

- Song: use the slide background when present, otherwise the Song default background; load the referenced asset and combine its relative path with the project root.
- Sermon: use the slide's image asset directly.
- Missing assets or failed lookups: leave the path empty so the card paints checkerboard.

The card loads or refreshes a scaled preview image based on its preview geometry. It never changes the persisted asset or slide record.

## Failure handling

- An empty cue list continues to show the existing authoring empty state.
- A missing, unreadable, or stale background file is non-fatal and produces checkerboard.
- An invalid or missing asset record is non-fatal for grid rendering and does not change program state.
- Resizing preserves cue order and live-cue identity.
- Existing context-menu actions remain available from the card's right-click path.

## Verification

Tests will cover:

- unchanged responsive column policy;
- card number, section label, summary, and footer-color mapping;
- checkerboard state when there is no background;
- usable background image state;
- 16:9 preview sizing;
- live-card highlighting and clearing;
- left-click live dispatch;
- right-click context dispatch;
- empty-state behavior.

The verification sequence is:

1. Run the targeted UI test executable or UI-labelled CTest lane.
2. Configure and build the debug preset.
3. Run the complete debug CTest suite.
4. Configure and build the release preset.
5. Run the complete release CTest suite.

The existing unrelated working-tree modification in `src/media/image_cache.cpp` and the untracked `issue 1.png` file are preserved and excluded from task commits.
