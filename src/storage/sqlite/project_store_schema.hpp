#pragma once

namespace shekinah::storage::project_store_internal {

inline constexpr char kSchema[] = R"sql(
CREATE TABLE IF NOT EXISTS schema_migrations(
    version INTEGER PRIMARY KEY,
    applied_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS projects(
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS assets(
    id TEXT PRIMARY KEY,
    kind TEXT NOT NULL,
    sha256 TEXT NOT NULL UNIQUE,
    relative_path TEXT NOT NULL,
    width INTEGER NOT NULL DEFAULT 0,
    height INTEGER NOT NULL DEFAULT 0,
    duration_ms INTEGER NOT NULL DEFAULT 0,
    archived INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS songs(
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    default_background_asset_id TEXT REFERENCES assets(id) ON DELETE SET NULL,
    archived INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS song_reflow_documents(
    song_id TEXT PRIMARY KEY REFERENCES songs(id) ON DELETE CASCADE,
    utf8_text TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS song_slide_breaks(
    song_id TEXT NOT NULL REFERENCES songs(id) ON DELETE CASCADE,
    ordinal INTEGER NOT NULL,
    utf8_offset INTEGER NOT NULL,
    section_label TEXT NOT NULL DEFAULT '',
    PRIMARY KEY(song_id, ordinal)
);
CREATE TABLE IF NOT EXISTS song_slides(
    id TEXT PRIMARY KEY,
    song_id TEXT NOT NULL REFERENCES songs(id) ON DELETE CASCADE,
    ordinal INTEGER NOT NULL,
    section_label TEXT NOT NULL DEFAULT '',
    lines_json TEXT NOT NULL,
    background_asset_id TEXT REFERENCES assets(id) ON DELETE SET NULL,
    UNIQUE(song_id, ordinal)
);
CREATE TABLE IF NOT EXISTS sermon_decks(
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    archived INTEGER NOT NULL DEFAULT 0
);
CREATE TABLE IF NOT EXISTS sermon_slides(
    id TEXT PRIMARY KEY,
    sermon_id TEXT NOT NULL REFERENCES sermon_decks(id) ON DELETE CASCADE,
    ordinal INTEGER NOT NULL,
    image_asset_id TEXT NOT NULL REFERENCES assets(id),
    UNIQUE(sermon_id, ordinal)
);
CREATE TABLE IF NOT EXISTS service_orders(
    id TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS service_order_items(
    id TEXT PRIMARY KEY,
    service_order_id TEXT NOT NULL REFERENCES service_orders(id) ON DELETE CASCADE,
    content_kind TEXT NOT NULL,
    content_id TEXT NOT NULL,
    ordinal INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS slide_backgrounds(
    slide_id TEXT PRIMARY KEY,
    asset_id TEXT NOT NULL REFERENCES assets(id)
);
CREATE TABLE IF NOT EXISTS settings(
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);
)sql";

}  // namespace shekinah::storage::project_store_internal
