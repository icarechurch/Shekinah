# Shekinah architecture migration handoff

This file is the durable continuation context for the full Shekinah
architecture migration. A new coding session must read this file and
`2026-11-26-architecture-migration-roadmap.md` before changing code.

## User directive

Execute the entire agreed refactor plan across the application. Do not stop at
the first few commits or merely produce another plan. Work in bounded,
coherent, test-passing commits until the feasible migration work is complete.

The rough 350–400 commit figure is a capacity estimate, not a quota. Commits
must represent real, reviewable architectural slices; never manufacture empty
or meaningless commits.

## Non-negotiable constraints

- Preserve all current UI/UX, including the committed Gemini-assisted UI
  improvements. Do not change layouts, colors, labels, focus behavior,
  keyboard behavior, drag/drop behavior, responsive behavior, or interaction
  semantics as part of architecture work.
- Keep the application a single-process native modular monolith. Use
  microservice-style ownership, contracts, and feature boundaries inside the
  process; do not split into runtime services.
- The 500-LOC target is a strong heuristic and hotspot report only. It is never
  a CI pass/fail rule.
- Typical-use working set target is 100 MB. Regular workflows must remain under
  300,000,000 bytes in tests; exceptional workloads may exceed that benchmark
  when unavoidable and must degrade gracefully.
- The exploratory whole-application 10 MB memory and 3% CPU goals are not
  promises. N4000 claims require later measurement on the Fujitsu Q5010
  (Celeron N4000, 4 GB RAM, 128 GB storage).
- Treat the current application and tests as the behavioral baseline. The new
  benchmark is deferred until the architecture work is sufficiently stable.
- Every commit must compile and pass its applicable tests. Do not weaken,
  skip, focus, or delete tests. Use TDD for new behavior and verify before
  claiming success.
- Do not use MeatLens-style architecture or assume C++ native expertise. Keep
  the migration incremental and explain ownership, lifetime, dependencies,
  threading, errors, invariants, and tests at each seam.

## Architecture decisions

The target is a strict feature-owned modular monolith:

```text
domain <- application <- storage
   ^          ^             ^
   |          |             |
   +-------- media ---------+
                ^
                |
                ui (composition root and feature adapters)
```

The enforced rules are documented in
`docs/architecture/architecture-boundaries.md` and implemented by
`cmake/ArchitecturePolicy.cmake`. The `shekinah_hotspots` target reports files
over 500 LOC without failing the build.

## Repository state at handoff

- Branch: `master`
- Latest local commit: `5a6047a refactor: isolate Song authoring`
- Worktree is clean at this handoff.
- The user confirmed that the previously accumulated migration history through
  `b7bd33d` was pushed before the current sprint. The migration commits after
  that push are local and should remain unpushed unless the user explicitly
  asks for a push.
- Do not reset, rebase, force-push, or discard user changes.

## Completed migration commits

1. `dc6d7d2 docs: define architecture migration roadmap`
2. `e6e5dae refactor: centralize song background resolution`
3. `e941393 refactor: centralize project session ownership`
4. `638bec3 refactor: extract cue presentation model`
5. `a9235d1 refactor: extract service order presentation`
6. `96d0d83 refactor: extract live slide presentation`
7. `7ee8d7c build: enforce architecture boundaries and report hotspots`
8. `3b2019e refactor: centralize sqlite support primitives`
9. `8d70603 refactor: isolate song persistence`

The first six commits extracted application/session/presentation seams while
preserving the UI. `7ee8d7c` added strict source-boundary checks and advisory
hotspot reporting. `3b2019e` moved SQLite statement/binding primitives into
`src/storage/sqlite/internal/sqlite_support.hpp`. The persistence migration now
also has typed Song, Sermon, Asset, Service Order, and archive adapters behind
the compatibility facade. Asset path resolution, importer ownership, and the
window resource-pool adapter are behind focused boundaries. The UI no longer
reaches `ProjectStore` or constructs storage importers directly, and
`ProjectSession` exposes typed repositories/import services instead of its
legacy store/repository accessors. Audience output no longer retains a
redundant decoded slide buffer in addition to the renderer's bounded image;
the existing loaded/cleared state contract is unchanged.

Since the previous handoff, the media and window seams were tightened further:

- `5a2810c` isolates slide background rendering from text rendering.
- `e0e99c6` isolates canonical text fitting and transformed text painting.
- `acc5643` isolates the audience video-player/widget lifecycle.
- `7c15037` centralizes default service-order identity and mutations behind a
  window-owned coordinator.
- `f94956b`, `c5d39ee`, and `4fac711` centralize aggregate persistence and
  presentation-registry refreshes across background, insertion, deletion,
  duplication, import, and editor flows.
- `27ea277` isolates the Library catalog projection.
- `85dbc77` isolates cue presentation loading and asset-path resolution.
- `0ed2083` isolates service-order view projection and removes the raw-repository
  view-builder seam.
- `3c79ae6` isolates live-media resolution behind the project session.
- `c5b33e2` repairs explicit `Result` wrapping in the cue presentation
  coordinator after the typed result boundary was tightened.
- `93c7161` isolates live-output presentation, including the existing image,
  video, missing-asset, and memory-admission branches.
- `89e5849` makes `ProjectSession` compose independent typed repository
  adapters instead of using the aggregate compatibility facade at runtime.
- `acb99cb` isolates Song and Sermon asset-slide insertion, including asset
  lookup, domain mutation, persistence, and returned slide-count projection.
- `7f62370` isolates Song asset background assignment, forward-range assignment,
  removal, and exact single-slide background persistence behind a typed
  coordinator.
- `76d8253` isolates Song and Sermon slide deletion and duplication, including
  cue lookup, minimum-slide protection, existing `*-copy-N` ID generation, and
  persistence. The window retains live-cue clearing and refresh policy.
- `eabfbf2` isolates slide inspection projection and Song-slide persistence,
  while the window retains Inspector widget invocation and refresh behavior.
- `67b737a` isolates asset importing and removes the raw AssetImporter accessor
  from the main-window header.
- `98c7e91` isolates reusable Sermon importing and removes the raw
  SermonDeckImporter accessor from the main-window header.
- `5a6047a` isolates reusable Song editing load/save policy, including Reflow
  loading, cue-ID preservation, background preservation, and appended-slide ID
  generation.

## Verification state

- The full debug gate currently passes 207/207 in 89.96 seconds, and the full
  release gate passes 207/207 in 90.03 seconds, with the architecture policy
  enabled.
- The focused storage gate passes 22/22 in both debug and release
  configurations after typed-session composition.
- The UI gate passes 122/122 in both debug and release configurations after
  the Song asset and slide-mutation extractions. The latest slide-mutation run
  completed in 68.25 seconds for debug and 63.95 seconds for release.
- The latest slide-inspection UI gate passes 122/122 in both debug and release
  configurations in 62.42 and 61.57 seconds respectively.
- The latest Song authoring UI gate passes 122/122 in both debug and release
  configurations in 62.92 and 61.87 seconds respectively.
- The current debug hotspot report says no source files exceed 500 LOC.
- The current media gate passes 19/19 in both debug and release configurations
  after the renderer and audience-video extractions.
- The current UI gate passes 122/122 in both debug and release configurations
  after the window coordinator, Song asset, and slide-mutation extractions.
- Build/test commands:

```text
cmake --preset windows-ucrt64-debug
cmake --build --preset windows-ucrt64-debug --parallel 2
ctest --test-dir build/windows-ucrt64-debug --output-on-failure

cmake --preset windows-ucrt64-release
cmake --build --preset windows-ucrt64-release --parallel 2
ctest --test-dir build/windows-ucrt64-release --output-on-failure

cmake --build --preset windows-ucrt64-debug --target shekinah_hotspots
```

## Exact continuation order

The SQLite aggregate responsibilities are now split behind typed contracts.
Continue with the next bounded architectural seams:

1. Reassess whether the compatibility `SqliteContentRepository` can be made
   internal to the composition root without weakening useful test contracts.
2. Extract application use-case/coordinator contracts from the remaining main
   window orchestration, preserving the current UI and interaction tests.
3. Decompose library, service order, cue grid, program status, editors, and
   theming by feature ownership without visual changes.
4. Split media renderer/output/cache/resource-pressure ownership.
5. Add trustworthy application-process measurement later, separate from the
   existing proxy benchmark.
6. Finish comprehensive architecture handbook documentation.
7. Perform final Fujitsu Q5010 hardware validation and record unmet targets
   honestly.

Continue through the roadmap:

- extract application use-case/coordinator contracts from `main_window.cpp`;
- decompose library, service order, cue grid, program status, editors, and
  theming by feature ownership without visual changes;
- split media renderer/output/cache/resource-pressure ownership;
- add trustworthy application-process measurement later, separate from the
  existing proxy benchmark;
- finish comprehensive architecture handbook documentation;
- perform final Fujitsu Q5010 hardware validation and record unmet targets
  honestly.

## Working protocol for every slice

1. Inspect the affected code and tests.
2. Add or update a focused failing test when behavior/policy is introduced.
3. Implement one bounded extraction or policy change.
4. Run the smallest complete debug and release checks for the affected target.
5. Run full gates at milestone boundaries and before completion claims.
6. Run `git diff --check`, inspect scope, commit a coherent slice, and verify
   the worktree.
7. Preserve unrelated user changes and never alter the established UI/UX.

The current next action is therefore: inspect the remaining compatibility
facade and main-window orchestration seams, select one bounded extraction,
test both configurations as applicable, and continue without creating a goal
or stopping at a planning checkpoint.
