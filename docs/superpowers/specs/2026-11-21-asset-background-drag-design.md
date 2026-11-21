# Asset Thumbnail Drag-to-Background Design

**Date:** 2026-11-21

**Status:** Approved design

## Goal

Present imported assets as visual thumbnail tiles and let an operator drag an asset onto a Song or a Song slide to assign background media.

## User-visible behavior

- The Assets tab uses a wrapping thumbnail grid rather than text-only rows.
- An image asset shows a bounded thumbnail and a compact filename label.
- A video asset shows a media placeholder icon and its filename label.
- The full relative path and byte size remain available in the tooltip.
- Dragging an asset carries its existing `AssetId` in a custom MIME payload.
- Dropping on a Song in the service order sets that Song's `default_background`.
- Dropping on a Song slide sets that asset on the dropped slide and every later slide in the Song.
- A later slide drop replaces the background for that slide and every slide after it.
- Sermon rows and Sermon slides do not accept asset background drops.
- The existing right-click `Set background` action remains a single-slide operation; only asset dragging uses forward propagation.

The UI will not invent date headings or other metadata that is not currently stored in the database. The visual reference is used for the thumbnail-card treatment, not as a requirement to fabricate asset dates.

## Architecture

### Drag contract

A shared UI helper owns the custom MIME type and the `AssetId` serialization/parsing rules. The asset list is the only drag source. Drop targets do not depend on list-row text or filesystem paths; they accept only a valid asset-id payload and resolve the asset through the repository before mutating a Song.

### Asset library

`LibraryDockWidget` keeps its current catalog ownership and filtering behavior. Its asset list switches to icon mode with wrapping, static movement, bounded icon dimensions, and a grid size suitable for several cards across the dock. Image thumbnails are read with `QImageReader` using auto-transform and a bounded scaled size. Video items use an existing Qt media icon because the current asset model does not store poster frames.

### Drop surfaces

`ServiceOrderWidget` reports a drop on the specific `ContentRef` under the pointer. `CueGridWidget` reports a drop on the specific `CueId` under the pointer. `OperatorPage` forwards the cue callback without persistence logic. Sermon content is rejected by the application boundary even if a malformed or future UI path reports a target.

### Background mutation

A small application-layer helper operates on an in-memory `domain::Song`:

1. Find the requested starting cue.
2. Return a not-found status without mutation if the cue is absent.
3. Assign the supplied `BackgroundRef` to the starting slide and every later slide.

Song default assignment remains a separate operation that sets `Song::default_background` and leaves per-slide overrides unchanged.

`ShekinahWindow` owns repository interaction. It validates the project, target content, and asset record; loads the target Song; applies the requested operation; saves the Song; registers the saved Song with the presentation service; and refreshes cues and live preview state. A failed validation or save leaves the stored Song unchanged.

## Data flow

```text
Asset tile
  -> custom MIME { AssetId }
  -> Service Order Song row OR Cue Grid Song card
  -> typed widget callback
  -> OperatorPage forwarding when needed
  -> ShekinahWindow repository mutation
  -> SQLite Song save
  -> presentation registration
  -> cue grid and live preview refresh
```

Dropping on a service-order Song does not need to select that Song. The callback carries the target content, allowing a non-selected Song's default background to be updated. Dropping on a cue card uses the currently displayed selected content and the card's cue id.

## Error handling and compatibility

- Missing project: reject with the existing storage failure status.
- Missing or invalid asset: reject before loading or saving the Song.
- Non-Song target: reject as invalid input with no mutation.
- Missing Song or cue: return the existing not-found error shape.
- Repository save failure: return the repository error and do not update the in-memory presentation registration.
- Existing SQLite schema and domain fields remain unchanged; no migration is required.

## Testing

The implementation follows red-green-refactor for each behavior slice.

Application tests cover:

- forward assignment from a valid cue;
- preservation of slides before the starting cue;
- invalid cue rejection without mutation;
- Song default assignment without overwriting slide overrides.

Widget tests cover:

- asset tile mode and thumbnail metadata;
- drag payload round-trip;
- service-order target reporting;
- cue-card target reporting;
- rejection boundaries for non-Song targets.

Session tests cover:

- a Song-row drop persisting the default background;
- a cue-card drop persisting the forward range;
- project reopen preserving the assigned range;
- cue and live-preview refresh after a successful assignment.

The final local verification mirrors the repository CI workflow:

```text
cmake --preset windows-ucrt64-debug
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug --output-on-failure
./build/windows-ucrt64-debug/shekinah_n4000_bench.exe --json shekinah-n4000.json
cmake --preset windows-ucrt64-release
cmake --build --preset windows-ucrt64-release --parallel 2
ctest --test-dir build/windows-ucrt64-release --output-on-failure
```

## Commit structure

The work will be delivered as at least 21 coherent commits, each containing a meaningful testable increment:

1. Approved design spec
2. Shared asset-drag contract
3. Asset tile-grid structure
4. Image thumbnail rendering
5. Video placeholder rendering
6. Asset labels and filtering in tile mode
7. Asset drag source
8. Service-order drop contract
9. Service-order drop implementation
10. Cue-grid drop contract
11. Cue-card drop implementation
12. Operator-page callback forwarding
13. Song slide-range behavior
14. Song default-background behavior
15. Main-window background mutation helpers
16. Service-order drop persistence wiring
17. Cue-slide drop persistence wiring
18. Preview and cue refresh integration
19. End-to-end persistence regressions
20. UI polish and accessibility details
21. Documentation and final cleanup

No empty commits, focused test filters, skipped tests, weakened assertions, or disabled CI lanes will be used.
