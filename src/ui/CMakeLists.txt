add_library(shekinah_ui STATIC
    app/main_window.cpp
    features/edit_song/reflow_editor_dialog.cpp
    features/edit_song/reflow_text_edit.cpp
    features/edit_sermon/sermon_editor_dialog.cpp
    features/inspect_slide/inspector_dialog.cpp
    pages/operator/operator_page.cpp
    support/app_icon.cpp
    support/include/shekinah/ui/support/app_icon.hpp
    support/include/shekinah/ui/support/theme.hpp
    support/theme.cpp
    ${PROJECT_SOURCE_DIR}/assets/icons/shekinah.qrc
    widgets/library_dock/library_dock_widget.cpp
    widgets/program_status/program_status_widget.cpp
    widgets/service_order/service_order_widget.cpp
    widgets/cue_grid/cue_grid_widget.cpp
)

target_include_directories(shekinah_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/cue_grid/include
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/program_status/include
        ${CMAKE_CURRENT_SOURCE_DIR}/pages/operator/include
        ${CMAKE_CURRENT_SOURCE_DIR}/features/inspect_slide/include
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/library_dock/include
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/service_order/include
        ${CMAKE_CURRENT_SOURCE_DIR}/support/include
        ${CMAKE_CURRENT_SOURCE_DIR}/features/edit_song/include
        ${CMAKE_CURRENT_SOURCE_DIR}/features/edit_sermon/include
        ${CMAKE_CURRENT_SOURCE_DIR}/app/include
)

target_link_libraries(shekinah_ui
    PUBLIC
        shekinah_application
        shekinah_domain
        shekinah_media
        shekinah_storage
        Qt6::Widgets
)

target_compile_features(shekinah_ui PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_ui)
