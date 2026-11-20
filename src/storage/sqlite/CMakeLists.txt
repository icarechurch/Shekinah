add_library(shekinah_storage STATIC
    project_store.cpp
)

target_include_directories(shekinah_storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shekinah_storage
    PUBLIC
        shekinah_domain
        SQLite3::SQLite3
)

target_compile_features(shekinah_storage PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_storage)
