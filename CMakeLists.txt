cmake_minimum_required(VERSION 3.28)

project(Shekinah VERSION 0.1.0 LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerWarnings.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
elseif (MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Widgets Multimedia Test)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
find_package(Catch2 3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include(CTest)
enable_testing()

add_executable(shekinah_app
    src/ui/app/main.cpp
)
target_link_libraries(shekinah_app PRIVATE Qt6::Widgets Qt6::Multimedia)

add_subdirectory(src/core/domain)
add_subdirectory(src/storage/sqlite)
add_subdirectory(src/media)
add_subdirectory(tests)
