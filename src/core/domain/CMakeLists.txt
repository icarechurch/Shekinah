add_library(shekinah_domain STATIC
    presentation_engine.cpp
)

target_include_directories(shekinah_domain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(shekinah_domain PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_domain)
