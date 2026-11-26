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
- Latest pushed commit: `8d70603 refactor: isolate song persistence`
- Worktree was clean after that commit.
- Do not reset, rebase, force-push, or discard user changes.
- The latest three commits after the previously pushed `96d0d83` were pushed
  by the user and are present on `origin/master`.

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
`src/storage/sqlite/internal/sqlite_support.hpp`. `8d70603` moved Song save,
load, Reflow load, and listing methods into
`src/storage/sqlite/song_repository.cpp` without changing the public
`SqliteContentRepository` API or schema.

## Verification state

- Before the latest persistence split, full debug and release gates passed
  207/207 with the architecture policy enabled.
- After `8d70603`, the focused storage gate passed 22/22 in both debug and
  release configurations.
- The next session must run the full debug/release gate before declaring the
  persistence milestone complete; do not infer it from the focused result.
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

Resume with the remaining `content_repository.cpp` responsibilities. Each
item is a separate coherent slice with focused storage tests, then full gates
at the persistence milestone:

1. Move Sermon save/load/list methods into `sermon_repository.cpp`.
2. Move Asset listing/loading into `asset_repository.cpp`.
3. Move service-order persistence into `service_order_repository.cpp`.
4. Keep archive/count behavior in a focused aggregate/lifecycle repository or
   split it into the smallest justified unit.
5. Reassess and document the resulting storage facade and contracts.

Then continue through the roadmap:

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

The current next action is therefore: read this handoff, inspect the remaining
Sermon methods in `content_repository.cpp`, extract them, test both storage
configurations, and continue without creating a goal or stopping at a planning
checkpoint.
