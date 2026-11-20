add_library(shekinah_media STATIC
    audience_output.cpp
    image_cache.cpp
    low_end_profile.cpp
    memory_diagnostics.cpp
)

target_include_directories(shekinah_media
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shekinah_media PUBLIC shekinah_domain Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
if (WIN32)
    target_link_libraries(shekinah_media PUBLIC psapi)
endif()
target_compile_features(shekinah_media PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_media)
