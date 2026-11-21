# Split Slide / Reflow Button Design

**Date:** 2026-11-21

**Status:** Approved for implementation

## Goal

Add a visible `Split Slide / Reflow` action to the bottom-left of the Song Reflow Editor dialog so operators can split lyrics at the current cursor without relying on `Ctrl+Enter`.

## Behavior

- The button is placed in the bottom dialog action row, on the left side.
- Save and Cancel remain on the right side and keep their current behavior.
- Clicking the button calls the same `ReflowTextEdit::insert_slide_break()` path used by `Ctrl+Enter`.
- The current cursor position is the split location.
- The lyric text is unchanged; only the visible slide-break marker and derived slide thumbnails change.
- Invalid cursor positions remain a no-op, matching the keyboard shortcut behavior.

## Scope

In scope: `ReflowEditorDialog`, its UI test, and the existing reflow editor test lane.

Out of scope: automatic blank-line/section splitting, changes to persistence, changes to keyboard shortcuts, changes to Save/Cancel, and changes to the operator cue grid.

## Verification

Add a Qt test that finds the bottom action button by object name, places the cursor, clicks the button, and verifies the document text is unchanged while the break count, slide count, and thumbnail count increase. Run the focused UI test, then the complete debug CTest suite.
