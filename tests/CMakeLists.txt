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
    storage/content_repository_test.cpp
    storage/project_store_test.cpp
    storage/asset_importer_test.cpp
    storage/sermon_deck_importer_test.cpp
)
target_include_directories(shekinah_storage_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests/support)
target_link_libraries(shekinah_storage_tests PRIVATE shekinah_storage Catch2::Catch2WithMain)
shekinah_register_test(shekinah_storage_tests storage)

add_executable(shekinah_media_tests
    media/audience_output_test.cpp
    media/image_cache_test.cpp
    media/low_end_profile_test.cpp
)
target_link_libraries(shekinah_media_tests PRIVATE shekinah_media Qt6::Widgets Catch2::Catch2WithMain)
shekinah_register_test(shekinah_media_tests media)

add_executable(shekinah_ui_tests
    ui/cue_grid_test.cpp
    ui/inspector_dialog_test.cpp
    ui/library_dock_test.cpp
    ui/library_search_test.cpp
    ui/main_window_test.cpp
    ui/operator_page_test.cpp
    ui/program_status_widget_test.cpp
    ui/reflow_editor_dialog_test.cpp
    ui/service_order_widget_test.cpp
)
target_link_libraries(shekinah_ui_tests PRIVATE shekinah_ui Qt6::Test Catch2::Catch2WithMain)
shekinah_register_test(shekinah_ui_tests ui)

add_executable(shekinah_performance_tests
    performance/memory_budget_test.cpp
)
target_link_libraries(shekinah_performance_tests PRIVATE shekinah_media Catch2::Catch2WithMain)
shekinah_register_test(shekinah_performance_tests performance)

add_executable(shekinah_application_tests
    application/presentation_service_test.cpp
    application/resource_governor_test.cpp
    application/service_order_service_test.cpp
    application/song_reflow_test.cpp
    application/song_text_importer_test.cpp
)
target_link_libraries(shekinah_application_tests PRIVATE shekinah_application Catch2::Catch2WithMain)
shekinah_register_test(shekinah_application_tests application)
