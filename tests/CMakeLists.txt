include(${PROJECT_SOURCE_DIR}/cmake/ShekinahTesting.cmake)

add_executable(shekinah_smoke_tests
    smoke/smoke_test.cpp
)
target_link_libraries(shekinah_smoke_tests PRIVATE Catch2::Catch2WithMain)
shekinah_register_test(shekinah_smoke_tests smoke)
