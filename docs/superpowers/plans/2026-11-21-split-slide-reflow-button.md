# Split Slide / Reflow Button Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a bottom-left `Split Slide / Reflow` button to the Song Reflow Editor that performs the existing cursor-based slide-break action.

**Architecture:** Keep `ReflowTextEdit::insert_slide_break()` as the single behavior source. Add one `QPushButton` to the dialog action row, connect it to that method, and rely on the existing document-changed callback to refresh thumbnails.

**Tech Stack:** C++20, Qt 6 Widgets, Catch2, QTest, CMake/Ninja, CTest.

## Global Constraints

- Preserve the existing `Ctrl+Enter` behavior.
- Preserve Save and Cancel behavior and placement on the right.
- Do not change persistence or domain/application models.
- Invalid cursor positions remain a no-op.
- Keep this change within two commits total: documentation, then implementation/tests.

---

### Task 1: Add and verify the bottom action

**Files:**
- Modify: `src/ui/features/edit_song/reflow_editor_dialog.cpp`
- Test: `tests/ui/reflow_editor_dialog_test.cpp`

**Interfaces:**
- Produces a `QPushButton` with object name `split-slide-reflow` and label `Split Slide / Reflow`.
- The button invokes `editor_->insert_slide_break()` and does not close the dialog.

- [ ] **Step 1: Write the failing UI test**

Add a test that sets text, places the cursor at offset 10, clicks `split-slide-reflow`, and asserts:

```cpp
REQUIRE(dialog.editor()->plain_text_utf8() == "First line\nSecond line");
REQUIRE(dialog.editor()->visible_break_count() == 1);
REQUIRE(dialog.reflow_document().to_slides().size() == 2);
REQUIRE(dialog.thumbnail_count() == 2);
```

- [ ] **Step 2: Run the focused test and verify the expected failure**

Run:

```powershell
cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests --parallel 2
ctest --test-dir build/windows-ucrt64-debug -R "Split Slide / Reflow" --output-on-failure
```

Expected: the test cannot find the `split-slide-reflow` button.

- [ ] **Step 3: Implement the button**

In `ReflowEditorDialog`, create a bottom action-row widget with a horizontal layout. Add the `Split Slide / Reflow` button to the left, add a stretch, then add the existing Save/Cancel `QDialogButtonBox` on the right. Connect the button as follows:

```cpp
QObject::connect(split_button, &QPushButton::clicked, this, [this] {
    editor_->insert_slide_break();
});
```

The existing `editor_->set_document_changed_handler([this] { refresh_thumbnails(); })` keeps the right-side thumbnails synchronized.

- [ ] **Step 4: Run focused and full UI verification**

Run:

```powershell
cmake --build --preset windows-ucrt64-debug --target shekinah_ui_tests --parallel 2
ctest --test-dir build/windows-ucrt64-debug -R "Split Slide / Reflow|Ctrl Enter inserts|Backspace at" --output-on-failure
ctest --test-dir build/windows-ucrt64-debug -L ui --output-on-failure
```

Expected: the new button test and all UI tests pass.

- [ ] **Step 5: Commit implementation and tests**

```powershell
git add src/ui/features/edit_song/reflow_editor_dialog.cpp tests/ui/reflow_editor_dialog_test.cpp
git commit -m "feat: add split slide reflow action"
```
