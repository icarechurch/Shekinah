# Shekinah README Design

## Goal

Replace the placeholder README with a project-specific README that explains what Shekinah is, who it serves, its current under-construction status, how to build and test it, and the constraints that shape its implementation.

## Audience

- I Care church staff and volunteers evaluating Shekinah as a worship-presentation tool.
- Developers setting up the native C++ project on Windows.
- Contributors reviewing the current MVP scope and low-end hardware requirements.

## Approved Approach

Keep the full section structure from the existing README template while replacing all placeholders with facts verified from the repository. The README will remain practical and concise rather than presenting Shekinah as production-ready.

## Content Design

### Positioning

The opening describes Shekinah as an under-construction ProPresenter alternative for I Care Balaybay Mountain Church and I Care SBMA. It explicitly states that I Care Center - main will continue using ProPresenter.

### Feature summary

Document only capabilities represented by the current code and tests:

- reusable Songs and service-order references;
- sermon decks imported from ordered PNG files;
- image and video backgrounds/assets;
- live cue selection and separate audience output;
- editing, reordering, duplication, and deletion of slides/cues;
- SQLite-backed project persistence and file-backed media;
- bounded resource behavior for Celeron N4000-class and 4 GB RAM targets.

### Technology and development

List the verified stack: C++20, CMake 3.28+, Ninja, Qt 6.7 (Widgets, Multimedia, and Test), SQLite 3, Catch2 3, nlohmann-json, MSYS2 UCRT64, and GitHub Actions. The setup instructions will follow `docs/development.md`, `CMakePresets.json`, and `.github/workflows/ci.yml`.

### Getting started and verification

Include MSYS2 UCRT64 prerequisites, package installation, debug and release configure/build commands, the complete CTest command, and the N4000 benchmark command. Explain that the full unfiltered CTest run is the local completion gate and that CI also builds/tests both debug and release configurations.

### Project structure and contribution guidance

Map the main `src`, `tests`, `tools`, `cmake`, and `docs` directories to their responsibilities. Explain that contributions should preserve the low-end resource budget, add or update tests for behavior changes, and run the documented checks.

### License and status

Do not invent a license. State that the repository does not currently declare one and that Shekinah is not yet a production release.

## Scope Boundaries

- Change `README.md` only after this design document is approved.
- Do not alter source code, build files, CI, or the existing development guide.
- Do not add unverified screenshots, download links, roadmap promises, or release claims.
- Do not change the user’s existing README content outside the requested replacement.

## Verification

After editing the README:

1. Scan it for template markers such as square-bracket placeholders and confirm none remain.
2. Compare commands and dependency names against the CMake presets, development guide, and CI workflow.
3. Run the repository’s relevant documentation/build checks; because the change is documentation-only, at minimum validate the README text and run the existing debug CTest gate if the configured environment supports it.
4. Inspect `git diff --check` and the final diff before reporting completion.
