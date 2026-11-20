add_library(shekinah_media STATIC
    image_cache.cpp
)

target_include_directories(shekinah_media
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shekinah_media PUBLIC shekinah_domain)
target_compile_features(shekinah_media PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_media)
