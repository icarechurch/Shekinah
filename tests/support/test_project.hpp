#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string_view>

class TestProject {
public:
    static TestProject create() {
        static std::atomic_uint64_t sequence{0};
        const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto suffix = std::to_string(now) + "-" + std::to_string(sequence.fetch_add(1));
        auto root = std::filesystem::temp_directory_path() / ("shekinah-test-" + suffix);
        std::filesystem::create_directories(root);
        return TestProject{std::move(root)};
    }

    TestProject(const TestProject&) = delete;
    TestProject& operator=(const TestProject&) = delete;
    TestProject(TestProject&& other) noexcept : root_(std::move(other.root_)) {
        other.root_.clear();
    }
    TestProject& operator=(TestProject&& other) noexcept {
        if (this != &other) {
            cleanup();
            root_ = std::move(other.root_);
            other.root_.clear();
        }
        return *this;
    }

    ~TestProject() { cleanup(); }

    [[nodiscard]] const std::filesystem::path& root() const noexcept { return root_; }

private:
    explicit TestProject(std::filesystem::path root) : root_(std::move(root)) {}

    void cleanup() noexcept {
        if (!root_.empty()) {
            std::error_code error;
            std::filesystem::remove_all(root_, error);
        }
    }

    std::filesystem::path root_;
};

inline std::filesystem::path fixture_path(std::string_view name) {
#ifdef SHEKINAH_FIXTURE_DIR
    return std::filesystem::path{SHEKINAH_FIXTURE_DIR} / std::string{name};
#else
    return std::filesystem::path{"tests/fixtures"} / std::string{name};
#endif
}
