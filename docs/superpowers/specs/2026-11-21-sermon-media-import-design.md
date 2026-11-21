# Sermon Media Import Design

## Goal

Allow Sermon decks to contain the same supported media formats as the general asset importer instead of restricting sermon slides to PNG files. Static images remain image slides, GIFs are accepted through the existing image path, and MP4 files are accepted as looping video slides when presented live.

## Scope

In scope:

- Replace the PNG-specific sermon importer API and validation with media-neutral naming and behavior.
- Accept PNG, JPG, JPEG, WEBP, BMP, GIF, TIFF, and MP4 files, case-insensitively.
- Update sermon import dialogs, labels, and errors to describe media/files rather than PNGs.
- Preserve the existing `AssetKind` routing, audience-output playback, and live-video resource admission rules.
- Add regression coverage for mixed supported media and unsupported extensions.

Out of scope:

- Adding pause/play controls to the sermon UI.
- Adding new animation behavior for GIFs; GIFs continue through the current image renderer.
- Changing the persisted sermon schema or the `SermonSlide` asset reference model.
- Adding content/MIME sniffing beyond the existing extension-based asset classification.

## Architecture and data flow

`SermonDeckImporter` will expose a media-neutral import method and will use `AssetImporter::import_file()` as the single source of truth for supported extensions and `AssetKind` classification. Each imported file remains an `AssetRecord`, and each sermon slide continues to store only the resulting `AssetId`; no database migration is required.

The UI file dialogs and editor will remove PNG-specific filters and wording. The existing general asset classifier will determine whether a selected file is an image or video. On live output, image assets continue through the image renderer and video assets continue through `QMediaPlayer`.

## Runtime behavior and errors

- Supported sermon media is PNG, JPG, JPEG, WEBP, BMP, GIF, TIFF, or MP4, regardless of extension case.
- GIFs are accepted as image assets and use the current decoded-image path.
- MP4s are accepted as video assets and play/loop through the existing audience-output player.
- Existing resource-governor admission rules for live video remain unchanged.
- Unsupported extensions return `unsupported_media`.
- Duplicate normalized filenames return `invalid_input`.
- Pause/play UI is not part of this change. `AudienceOutput::set_video_paused()` remains the future control point.

## Verification

Add or update tests to:

1. Import a mixed sermon deck and verify the imported slide assets retain image/video classification, including GIF and MP4 extensions.
2. Verify unsupported sermon extensions still fail with `unsupported_media`.
3. Verify the sermon editor uses media-neutral labels and accepts arbitrary selected file paths.

Run targeted storage and UI tests first. Then run the complete debug build and unfiltered CTest gate, followed by the release build and unfiltered CTest gate documented in `docs/development.md`. If the local Windows UCRT64 environment is unavailable, report those checks as unverified.

## Acceptance criteria

- A user can select supported image or MP4 files in the sermon import/create/edit flow without PNG-only filtering.
- Mixed supported media imports successfully into one Sermon deck.
- A live MP4 sermon slide uses the existing looping video output path.
- Existing PNG sermon behavior, ordering, duplicate detection, persistence, and resource limits remain intact.
- Unsupported extensions continue to fail cleanly.
- All available relevant local CI checks pass, with unavailable checks explicitly identified.
