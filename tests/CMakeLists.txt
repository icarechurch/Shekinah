include(${PROJECT_SOURCE_DIR}/cmake/ShekinahTesting.cmake)

add_executable(shekinah_smoke_tests
    smoke/smoke_test.cpp
)
target_link_libraries(shekinah_smoke_tests PRIVATE Catch2::Catch2WithMain)
shekinah_register_test(shekinah_smoke_tests smoke)

add_executable(shekinah_test_support_tests
    support/test_project_test.cpp
)
target_include_directories(shekinah_test_support_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/support)
target_link_libraries(shekinah_test_support_tests PRIVATE Catch2::Catch2WithMain)
shekinah_register_test(shekinah_test_support_tests support)
