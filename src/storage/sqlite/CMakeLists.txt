add_library(shekinah_storage STATIC
    asset_importer.cpp
    asset_importer_file.cpp
    asset_repository.cpp
    content_repository.cpp
    project_store.cpp
    project_store_schema.hpp
    project_store_transactions.cpp
    project_session.cpp
    sermon_deck_importer.cpp
    sermon_deck_importer_order.cpp
    sermon_repository.cpp
    sermon_repository_listing.cpp
    sermon_repository_writes.cpp
    service_order_repository_append.cpp
    service_order_repository_mutations.cpp
    service_order_repository_ordering.cpp
    service_order_repository_reads.cpp
    song_repository_reads.cpp
    song_repository_listing.cpp
    song_repository_reflow.cpp
    song_repository_write_reflow.cpp
    song_repository_write_slides.cpp
    song_repository_writes.cpp
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
