add_library(shekinah_storage STATIC
    asset_importer.cpp
    asset_repository.cpp
    content_repository.cpp
    project_store.cpp
    project_session.cpp
    sermon_deck_importer.cpp
    sermon_repository.cpp
    song_repository.cpp
)

target_include_directories(shekinah_storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shekinah_storage
    PUBLIC
        shekinah_domain
        shekinah_application
        SQLite3::SQLite3
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)

target_compile_features(shekinah_storage PUBLIC cxx_std_20)
shekinah_enable_warnings(shekinah_storage)
