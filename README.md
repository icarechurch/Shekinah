# Shekinah

> *"Then the cloud covered the tent of meeting, and the glory of the Lord filled the tabernacle."* — Exodus 40:34

**Shekinah** (`שְׁכִינָה` — Hebrew for "dwelling" or "settling," referring to the tangible, abiding presence of God) is an **under-construction ProPresenter alternative** for **I Care Balaybay Mountain Church** and **I Care SBMA**.

Shekinah is being built to keep worship presentation practical on modest Windows hardware, including Celeron-class computers with 4 GB of RAM. **I Care Center - main will continue using ProPresenter.**

---

## Table of Contents

- [About the Project](#about-the-project)
- [Key Features](#key-features)
- [Tech Stack](#tech-stack)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
  - [Environment Variables](#environment-variables)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

---

## About the Project

Shekinah is a lightweight native desktop application for preparing and operating church worship presentations. It combines reusable songs, sermon decks, media assets, a service order, live cue control, and a separate audience-output window in one Windows-first application.

The project is intentionally focused on the needs of I Care Balaybay Mountain Church and I Care SBMA. It is still under construction, so behavior and interfaces may change as the MVP develops.

---

## Key Features

- **Songs and service order:** Create or import Songs, reuse content, and arrange service items for a presentation.
- **Sermon decks:** Import ordered image or video media as Sermon decks and edit their slide content.
- **Media assets:** Import image and video assets, use them as Song or slide backgrounds, and keep project media file-backed.
- **Live cue control:** Select cues from the operator workspace, navigate presentation state, and manage clear or black output states.
- **Audience output:** Show the live presentation in a separate audience-output window.
- **Slide editing:** Inspect, edit, duplicate, reorder, and delete Song or Sermon slides where supported by the current UI.
- **Local project storage:** Persist project content and metadata in SQLite while keeping media files in the project asset store.
- **Low-end resource controls:** Bound image and video handling for Celeron N4000-class hardware and modest memory budgets.

---

## Tech Stack

- **Language:** C++20
- **Build:** CMake 3.28+ and Ninja
- **UI and media:** Qt 6.7 Widgets, Multimedia, MultimediaWidgets, and Test
- **Persistence:** SQLite 3 with file-backed project assets
- **Testing:** Catch2 3, CTest, and Qt Test
- **JSON and benchmark support:** nlohmann-json
- **Windows development and CI:** MSYS2 UCRT64 and GitHub Actions

---

## Getting Started

Shekinah currently targets Windows through the MSYS2 UCRT64 environment. The repository presets use `C:/msys64/ucrt64/bin/g++.exe` and `C:/msys64/ucrt64` as the package prefix.

### Prerequisites

Install the following before configuring the project:

- Windows with [MSYS2](https://www.msys2.org/)
- CMake 3.28 or newer
- Ninja
- GCC/G++ for the UCRT64 environment
- Qt 6.7 packages for Core, Gui, Widgets, Multimedia, MultimediaWidgets, and Test
- SQLite 3
- Catch2 3
- nlohmann-json

From PowerShell, update MSYS2 and install the project dependencies:

```powershell
& C:\msys64\usr\bin\bash.exe -lc 'pacman -Syu --noconfirm'
& C:\msys64\usr\bin\bash.exe -lc 'pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-multimedia mingw-w64-ucrt-x86_64-qt6-multimedia-widgets mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-catch mingw-w64-ucrt-x86_64-nlohmann-json'
```

### Installation

Clone the repository and configure a debug build with the checked-in CMake preset:

```powershell
git clone https://github.com/icarechurch/Shekinah.git
cd Shekinah
cmake --preset windows-ucrt64-debug
cmake --build --preset windows-ucrt64-debug
```

The debug executable is created at `build/windows-ucrt64-debug/shekinah_app.exe`.

### Environment Variables

Shekinah does not currently require application-specific environment variables. Configuration is provided through the CMake presets and the MSYS2 UCRT64 installation described above.

---

## Usage

Launch the debug build from PowerShell:

```powershell
.\build\windows-ucrt64-debug\shekinah_app.exe
```

The current operator flow is:

1. Create or open a project folder.
2. Create or import Songs and Sermon decks from the library.
3. Import image or video assets as needed.
4. Arrange Songs and Sermons in the service order.
5. Select a cue in the presentation workspace to send it live.
6. Use **Show Audience Output** to open the separate output window.

For a release build and the complete local test gate:

```powershell
cmake --preset windows-ucrt64-release
cmake --build --preset windows-ucrt64-release
ctest --test-dir build/windows-ucrt64-release --output-on-failure
```

The debug test gate is:

```powershell
ctest --test-dir build/windows-ucrt64-debug --output-on-failure
```

### Low-End Verification

The minimum performance target is an Intel Celeron N4000-class machine with UHD 600 graphics and 4 GB of RAM. The runtime bounds still-image decoding to 1920×1080, permits one active loop video, samples process working set, and stops admitting new live video at 270,000,000 bytes. The hard process-memory gate is 300,000,000 bytes.

Run the dedicated benchmark after building the debug configuration:

```powershell
.\build\windows-ucrt64-debug\shekinah_n4000_bench.exe --json shekinah-n4000.json
```

The benchmark report records working set, measured cue-latency p95, CPU time, decode-path policy, and resource-governor transitions.

---

## Project Structure

```text
cmake/                 Shared CMake warnings and CTest configuration
docs/                  Development notes, designs, and implementation plans
src/core/domain/       Qt-independent presentation and content models
src/core/application/  Application services for content and presentation workflows
src/media/             Audience output, rendering, caching, and low-end resource controls
src/storage/sqlite/    SQLite project storage and asset import
src/ui/                Qt application shell, operator page, editors, and widgets
tests/                 Catch2, Qt Test, integration, end-to-end, and performance tests
tools/n4000_bench/     Celeron N4000-oriented benchmark executable
```

---

## Contributing

Shekinah is an active MVP project. When changing behavior:

- Keep the application usable on the low-end hardware target.
- Add or update tests for the behavior being changed.
- Run the full, unfiltered CTest command for the configuration you changed.
- Run the N4000 benchmark when changing media loading, rendering, or resource-governor behavior.
- Keep the Qt UI, application services, domain model, SQLite storage, and media responsibilities separated along the existing project boundaries.

The GitHub Actions workflow builds and tests both debug and release configurations, and uploads the N4000 benchmark report.

---

## License

No license file or license declaration is currently present in this repository. Licensing information will be added by the project maintainers.
