#pragma once

#include <string>
#include <utility>
#include <variant>

namespace shekinah::domain {

enum class ErrorCode {
    invalid_input,
    not_found,
    io_failure,
    storage_failure,
    unsupported_media,
    copy_failed,
};

struct Error {
    ErrorCode code;
    std::string message;
};

template <typename T>
class Result {
public:
    static Result ok(T value) { return Result{std::move(value)}; }
    static Result fail(Error error) { return Result{std::move(error)}; }

    explicit operator bool() const noexcept { return std::holds_alternative<T>(value_); }

    T& value() { return std::get<T>(value_); }
    const T& value() const { return std::get<T>(value_); }
    T* operator->() { return &value(); }
    const T* operator->() const { return &value(); }
    T& operator*() { return value(); }
    const T& operator*() const { return value(); }

    const Error& error() const { return std::get<Error>(value_); }

private:
    explicit Result(T value) : value_(std::move(value)) {}
    explicit Result(Error error) : value_(std::move(error)) {}

    std::variant<T, Error> value_;
};

using Status = Result<std::monostate>;

inline Status success() { return Status::ok(std::monostate{}); }

}  // namespace shekinah::domain
