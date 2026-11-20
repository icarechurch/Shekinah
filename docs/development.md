# Shekinah development

## Windows UCRT64 setup

Install MSYS2, then run from PowerShell:

```powershell
& C:\msys64\usr\bin\bash.exe -lc 'pacman -Syu --noconfirm'
& C:\msys64\usr\bin\bash.exe -lc 'pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-multimedia mingw-w64-ucrt-x86_64-qt6-multimedia-widgets mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-catch mingw-w64-ucrt-x86_64-nlohmann-json'
```

The repository presets use `C:/msys64/ucrt64/bin/g++.exe` and `C:/msys64/ucrt64` as the Qt/package prefix.

## Build and test

```powershell
cmake --preset windows-ucrt64-debug
cmake --build --preset windows-ucrt64-debug
ctest --test-dir build/windows-ucrt64-debug --output-on-failure
```

Use the release preset for performance checks:

```powershell
cmake --preset windows-ucrt64-release
cmake --build --preset windows-ucrt64-release
ctest --test-dir build/windows-ucrt64-release --output-on-failure
```

CTest labels are `smoke`, `support`, `domain`, `storage`, `application`, `media`, `ui`, `e2e`, and `performance`. Targeted checks are useful during development, but the full unfiltered command is the completion gate.

## Low-end verification

The minimum target is an Intel Celeron N4000-class machine with UHD 600 graphics. The runtime keeps still-image decode bounded to 1920×1080, permits one active loop video, samples the process working set every second, and stops admitting new live video at 270,000,000 bytes. The hard performance gate is 300,000,000 bytes.

Run the end-to-end and performance lanes directly when tuning a low-end build:

```powershell
ctest --test-dir build/windows-ucrt64-debug -L e2e --output-on-failure
ctest --test-dir build/windows-ucrt64-debug -L performance --output-on-failure
.\build\windows-ucrt64-debug\shekinah_n4000_bench.exe --json shekinah-n4000.json
```

The benchmark report records working set, measured cue latency p95, CPU time, decode-path policy, and governor transitions. CI uploads this JSON report as the `shekinah-n4000-benchmark` artifact.
