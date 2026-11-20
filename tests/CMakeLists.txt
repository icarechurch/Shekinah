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

add_executable(shekinah_domain_tests
    domain/presentation_engine_test.cpp
)
target_link_libraries(shekinah_domain_tests PRIVATE shekinah_domain Catch2::Catch2WithMain)
shekinah_register_test(shekinah_domain_tests domain)

target_sources(shekinah_domain_tests PRIVATE domain/content_model_test.cpp)
target_sources(shekinah_domain_tests PRIVATE domain/result_test.cpp)

add_executable(shekinah_storage_tests
    storage/project_store_test.cpp
    storage/asset_importer_test.cpp
)
target_include_directories(shekinah_storage_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests/support)
target_link_libraries(shekinah_storage_tests PRIVATE shekinah_storage Catch2::Catch2WithMain)
shekinah_register_test(shekinah_storage_tests storage)

add_executable(shekinah_media_tests
    media/image_cache_test.cpp
)
target_link_libraries(shekinah_media_tests PRIVATE shekinah_media Catch2::Catch2WithMain)
shekinah_register_test(shekinah_media_tests media)
