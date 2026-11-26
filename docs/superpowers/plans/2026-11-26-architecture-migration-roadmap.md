# Shekinah Architecture Migration Roadmap

**Goal:** Migrate Shekinah incrementally to strict, feature-owned native architecture while preserving operator and audience behavior, project compatibility, and low-end usability.

**Architecture:** Keep a single native process with a composition root, domain and application policies, focused feature UI, and infrastructure adapters. Feature ownership and narrow interfaces are enforced at build and source boundaries; process separation is not required.

**Tech stack:** C++20, Qt 6, CMake, SQLite, Catch2, Windows UCRT64.

## Constraints

- Preserve the committed UI/UX, interaction behavior, existing tests, and project data. Intentional behavior changes need separate approval.
- Treat 500 lines per file as a heuristic and report hotspots; never make it a CI pass/fail rule.
- Preserve existing gates while introducing documented, ratcheting baselines for pre-existing static-analysis findings.
- Each commit must compile and pass applicable tests; run the full relevant local CI gate before completing a milestone. Keep test lanes under 110 seconds through deterministic, coverage-preserving improvements.
- Target 100 MB working set in typical use; test that defined regular workflows stay below 300,000,000 bytes. Exceptional workloads may exceed that value and must be measured and handled gracefully.
- Investigate 10 MB total memory and 3% CPU as exploratory goals, not promises. N4000-specific claims require measurement on the target device.
- Explain C++ ownership, lifetime, threading, dependencies, invariants, errors, and tests as each module migrates. Complete the architecture handbook after migration.
- The rough 350–400 commit estimate is a capacity estimate, not a quota.

## Milestones

1. **Existing baseline and characterization:** treat the current committed application and existing tests as the behavioral baseline. Add focused characterization when a migration slice needs it; do not block refactoring on a new benchmark. Preserve existing tests until equivalent or stronger coverage exists.
2. **Enforcement baseline:** document and enforce target dependency rules; introduce reproducible formatting, static analysis and include hygiene with a ratchet for existing findings. Report file-size hotspots without a limit gate.
3. **Project/session pilot:** characterize session lifecycle, then extract focused session coordination and composition while preserving UI, project files, errors and live state.
4. **Domain and application ownership:** clarify presentation and service-order state and policy ownership; introduce only justified narrow contracts with independent tests.
5. **Persistence:** split responsibilities by aggregate, isolate SQLite mechanics, test transactions, errors and project compatibility.
6. **Feature migration:** decompose service order, cues, library, program status, editors and theming into independently understandable features; preserve visuals, focus, keyboard, drag/drop and responsive behavior.
7. **Media and resource management:** clarify renderer, media, cache and pressure ownership; establish trustworthy memory and responsiveness measurements later; protect live output when shedding resources.
8. **Documentation and final hardware validation:** finish the architecture handbook and run the same benchmark workloads on the Fujitsu N4000 tablet. Record any unmet targets rather than presenting proxy measurements as hardware results.

Each milestone needs a separate, evidence-based implementation plan with exact files, test cases and verification commands. Reassess the estimate after the pilot rather than forcing a commit count.

## Initial measurement contract

The existing `shekinah_n4000_bench` exercises an in-memory service order rather than the running application. Its field named `peak_working_set_bytes` is a single sample; `cue_latency_p95_ms` measures appends, `cpu_percent_p95` is an overall CPU-time ratio, and `frame_time_p95_ms` is a constant. These fields must not be interpreted as measured application performance. The existing performance test samples its own test process and does not prove the application stays below 300 MB.

When measurement work begins later, it must record executable/build identity, Windows version, hardware, display setup, project fixture and workload. Measure the application process over time: current and peak working set, private committed bytes, normalized CPU and actual cue latency, with the sampling interval and duration recorded. Distinguish startup, idle, static audience output, editing/cue changes, video and pressure recovery. Do not fabricate missing metrics or use process working-set trimming to manufacture a low result.

Use a dedicated disposable project with a positively verified path; changing `APPDATA` alone did not establish isolation in the earlier exploratory run. Do not launch measurements against a user's project without explicit intent. Keep real hardware results separate from development-machine proxies.
