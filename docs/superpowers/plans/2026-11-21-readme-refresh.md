# Shekinah README Refresh Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the placeholder README with an accurate, full-template project README for the under-construction Shekinah worship-presentation application.

**Architecture:** This is a documentation-only change. `README.md` will be the public overview and quick-start entry point, while `docs/development.md`, `CMakePresets.json`, and `.github/workflows/ci.yml` remain the authoritative sources for detailed development commands and CI behavior.

**Tech Stack:** Markdown, C++20, CMake 3.28+, Ninja, Qt 6.7, SQLite 3, Catch2 3, nlohmann-json, MSYS2 UCRT64, GitHub Actions.

## Global Constraints

- Describe Shekinah as under construction and intended for I Care Balaybay Mountain Church and I Care SBMA.
- State that I Care Center - main will continue using ProPresenter.
- Document only capabilities verified in the source tree, tests, development guide, CMake presets, or CI workflow.
- Preserve the full template sections: About the Project, Key Features, Tech Stack, Getting Started, Usage, Project Structure, Contributing, and License.
- Do not invent a license, release status, download link, screenshot, roadmap promise, or unsupported platform guarantee.
- Do not change source code, build files, CI, or `docs/development.md`.

---

### Task 1: Rewrite the project README

**Files:**
- Modify: `README.md`

**Interfaces:**
- Consumes: Project facts from `CMakeLists.txt`, `CMakePresets.json`, `docs/development.md`, `.github/workflows/ci.yml`, `src/`, `tests/`, and `tools/n4000_bench/main.cpp`.
- Produces: A placeholder-free README that a church operator can use to understand the project and a contributor can use to build and test it.

- [x] **Step 1: Replace the opening and table of contents**

Use the existing title and verse, then describe Shekinah as an under-construction ProPresenter alternative for I Care Balaybay Mountain Church and I Care SBMA. Include the explicit note that I Care Center - main will continue using ProPresenter. Keep table-of-contents links aligned with the final headings, including nested links for prerequisites, installation, and environment variables.

- [x] **Step 2: Fill in the project overview and feature sections**

Explain that the application is a lightweight worship-presentation desktop tool focused on live service operation, reusable content, audience output, and Celeron/4 GB RAM practicality. List only these verified capabilities:

```text
- reusable Songs and service-order references;
- sermon decks imported from ordered PNG files;
- image and video backgrounds/assets;
- live cue selection and a separate audience-output window;
- slide/cue editing, reordering, duplication, and deletion;
- SQLite-backed project persistence with file-backed media;
- bounded resource behavior for Celeron N4000-class hardware.
```

- [x] **Step 3: Document the verified technology stack**

Use concrete stack entries rather than generic frontend/backend labels:

```text
- Language: C++20
- Build: CMake 3.28+ and Ninja
- UI and media: Qt 6.7 Widgets, Multimedia, MultimediaWidgets, and Test
- Persistence: SQLite 3 with file-backed project assets
- Testing: Catch2 3, CTest, and Qt Test
- JSON/benchmark support: nlohmann-json
- Windows development and CI: MSYS2 UCRT64 and GitHub Actions
```

- [x] **Step 4: Add accurate prerequisites and installation commands**

Document Windows UCRT64 prerequisites: MSYS2, CMake 3.28+, Ninja, GCC/G++, Qt 6.7 packages, SQLite 3, Catch2 3, and nlohmann-json. Include the repository’s existing PowerShell MSYS2 package commands from `docs/development.md`, followed by:

```powershell
git clone https://github.com/icarechurch/Shekinah.git
cd Shekinah
cmake --preset windows-ucrt64-debug
cmake --build --preset windows-ucrt64-debug
```

Use the verified repository URL `https://github.com/icarechurch/Shekinah.git`. State that the project does not currently require application environment variables and therefore has no environment-variable setup step.

- [x] **Step 5: Add usage, verification, and low-end guidance**

Explain that the debug build produces `shekinah_app`, that the release preset is intended for performance checks, and that the benchmark target is `shekinah_n4000_bench`. Include:

```powershell
ctest --test-dir build/windows-ucrt64-debug --output-on-failure
cmake --preset windows-ucrt64-release
cmake --build --preset windows-ucrt64-release
ctest --test-dir build/windows-ucrt64-release --output-on-failure
.\build\windows-ucrt64-debug\shekinah_n4000_bench.exe --json shekinah-n4000.json
```

Describe the benchmark as recording working set, cue-latency p95, CPU time, decode policy, and governor transitions. Mention the Celeron N4000/UHD 600 target, bounded still-image decode, one active loop video, and the documented 270,000,000-byte admission threshold with a 300,000,000-byte hard gate without presenting those numbers as a general system requirement.

- [x] **Step 6: Add structure, contribution, and license/status sections**

Map `src/core`, `src/media`, `src/storage`, `src/ui`, `tests`, `tools`, `cmake`, and `docs` to their responsibilities. Tell contributors to preserve the low-end resource budget, add or update tests for behavior changes, run the full unfiltered CTest command, and run the N4000 benchmark when changing media or resource behavior. State that no license file or license declaration is currently present and that the project is not a production release.

- [x] **Step 7: Inspect the README diff**

Run:

```powershell
git diff -- README.md
git diff --check
```

Expected result: the diff contains only the intended README rewrite and `git diff --check` emits no whitespace errors.

### Task 2: Validate the documentation against the repository gates

**Files:**
- Read: `README.md`
- Read: `docs/development.md`
- Read: `CMakePresets.json`
- Read: `.github/workflows/ci.yml`

**Interfaces:**
- Consumes: The completed README and the repository’s documented debug/release commands.
- Produces: Fresh verification evidence that the README has no template placeholders and that its commands and dependency names match the project.

- [x] **Step 1: Scan for unresolved template content**

Run:

```powershell
rg -n 'e\.g\.|TBD|TODO|FIXME|<repository-url>|\[Feature [0-9]|Short description|brief one-sentence|core purpose' README.md
```

Expected result: no matches.

- [x] **Step 2: Check command and dependency consistency**

Compare every command and package name in `README.md` with `docs/development.md`, `CMakePresets.json`, and `.github/workflows/ci.yml`. Correct any mismatch before continuing; do not weaken or omit the CI-required debug/release test commands.

- [x] **Step 3: Run the existing debug test gate when configured**

If `build/windows-ucrt64-debug` is configured, run:

```powershell
ctest --test-dir build/windows-ucrt64-debug --output-on-failure
```

Expected result: all registered tests pass. If the environment is not configured, record that the repository test gate could not be run locally rather than claiming it passed.

- [x] **Step 4: Confirm final scope and status**

Run:

```powershell
git status --short
git diff --stat
git diff --check
```

Expected result: only `README.md` is modified after the already committed design and plan documents, with no whitespace errors and no source/build/CI changes.
