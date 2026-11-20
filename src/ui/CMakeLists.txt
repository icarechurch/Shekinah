add_library(shekinah_ui STATIC
    widgets/cue_grid/cue_grid_widget.cpp
)

target_include_directories(shekinah_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/cue_grid/include
)

target_link_libraries(shekinah_ui
    PUBLIC
        shekinah_domain
        Qt6::Widgets
)

target_compile_features(shekinah_ui PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_ui)
