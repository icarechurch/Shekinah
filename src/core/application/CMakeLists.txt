add_library(shekinah_application STATIC
    service_order_service.cpp
    song_reflow.cpp
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
