# Architecture boundaries

Shekinah is a single-process native modular monolith. The build graph expresses
the dependency direction; features are decomposed by ownership and contracts,
not by spawning separate processes.

## Dependency direction

```text
domain <- application <- storage
   ^          ^             ^
   |          |             |
   +-------- media ---------+
                ^
                |
                ui (composition root and feature adapters)
```

The practical rules are:

- `domain` contains portable business models and policies. It cannot include
  Qt, application, storage, media, or UI headers.
- `application` contains use-case and presentation policies. It can depend on
  `domain`, but not Qt, storage, media, or UI.
- `storage` contains persistence and import adapters. It can depend inward on
  domain/application contracts, but cannot depend on media, Qt, or UI.
- `media` contains rendering, output, cache, and resource-pressure adapters. It
  can depend on domain and Qt, but not application, storage, or UI.
- `ui` is the outer composition layer. It may adapt application, storage, and
  media contracts into Qt widgets and feature interactions.

These rules are enforced during CMake configuration by
`cmake/ArchitecturePolicy.cmake`. A violation is a blocking build/configuration
failure.

## File-size heuristic

The 500-LOC target is a strong decomposition heuristic, not a CI pass/fail
rule. Run the advisory report with:

```text
cmake --build --preset windows-ucrt64-debug --target shekinah_hotspots
```

Hotspots should be decomposed when ownership, change cadence, testability, or
dependency clarity improves by doing so. Generated code, narrowly scoped UI
layout code, and migration seams may temporarily exceed the heuristic and must
be recorded in the migration plan rather than hidden or artificially split.

## Ownership and lifetime

- The composition root owns long-lived application services and adapters.
- Feature widgets own presentation state only; persistence and media ownership
  stay behind explicit boundaries.
- Borrowed domain data is valid only for the scope documented by the caller.
  Application presenters must not retain pointers or references to storage
  result values.
- Resource pressure may shed non-live caches, but must not invalidate the live
  output contract.

Every migrated module should add focused tests for its policy, lifetime,
failure behavior, and integration boundary before the old implementation is
removed.
