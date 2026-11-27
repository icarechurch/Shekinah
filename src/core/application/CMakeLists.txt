add_library(shekinah_application STATIC
    cue_presenter_song.cpp
    cue_presenter_sermon.cpp
    live_slide_presenter.cpp
    presentation_service.cpp
    presentation_service_backgrounds.cpp
    presentation_service_navigation.cpp
    presentation_service_live_navigation.cpp
    resource_governor.cpp
    song_authoring.cpp
    service_order_service.cpp
    service_order_presenter.cpp
    slide_insertion.cpp
    song_backgrounds.cpp
    song_reflow.cpp
    song_reflow_projection.cpp
    song_text_importer.cpp
)

target_include_directories(shekinah_application
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shekinah_application
    PUBLIC
        shekinah_domain
)

target_compile_features(shekinah_application PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_application)
