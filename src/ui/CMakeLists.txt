add_library(shekinah_ui STATIC
    features/inspect_slide/inspector_dialog.cpp
    pages/operator/operator_page.cpp
    widgets/library_dock/library_dock_widget.cpp
    widgets/program_status/program_status_widget.cpp
    widgets/cue_grid/cue_grid_widget.cpp
)

target_include_directories(shekinah_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/cue_grid/include
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/program_status/include
        ${CMAKE_CURRENT_SOURCE_DIR}/pages/operator/include
        ${CMAKE_CURRENT_SOURCE_DIR}/features/inspect_slide/include
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/library_dock/include
)

target_link_libraries(shekinah_ui
    PUBLIC
        shekinah_domain
        Qt6::Widgets
)

target_compile_features(shekinah_ui PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_ui)
