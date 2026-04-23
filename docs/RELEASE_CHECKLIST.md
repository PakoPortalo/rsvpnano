# Maintainer Release Checklist

Use this checklist before publishing a release or announcing a new firmware build.

## Firmware

- Build the default firmware: `pio run`
- Export the browser-flasher artifact: `python3 tools/export_web_firmware.py`
- Flash the release firmware and confirm boot, display, touch, SD mount, battery readout, and USB transfer mode.
- Open at least one existing `.rsvp` book.
- Convert at least one fresh `.epub` on-device and confirm the resulting `.rsvp` opens correctly.
- Re-open the converted book and confirm the library prefers the cached `.rsvp` file.

## SD Card And Books

- Confirm `/books` exists on the SD card.
- Confirm interrupted-conversion markers such as `.rsvp.tmp`, `.rsvp.failed`, and `.rsvp.converting` are cleared after a successful retry.
- Test the desktop converter in `tools/sd_card_converter` with at least one EPUB.

## Repository

- Confirm `git status --short` only contains intentional changes.
- Run `git diff --check`.
- Confirm no sample books, local `.epub` files, generated `.rsvp` files, or PlatformIO build outputs are staged.
- Confirm `README.md` still matches the current hardware, build, and flashing flow.
- Confirm the GitHub Pages deploy completes and the web flasher opens over HTTPS.

## Suggested Publish Flow

```sh
git status --short
git diff --check
pio run
python3 tools/export_web_firmware.py
git add .
git commit -m "Prepare release"
git push origin HEAD
```
