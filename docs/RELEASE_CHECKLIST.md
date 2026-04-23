# Release Checklist

Use this before pushing a public milestone or tagging a release.

## Firmware

- Build default firmware: `pio run`
- Build USB transfer firmware: `pio run -e waveshare_esp32s3_usb_msc`
- Flash the default firmware and confirm boot, display, touch, SD mount, and battery readout.
- Open at least one cached `.rsvp` book.
- Convert at least one fresh `.epub` on-device and confirm the resulting `.rsvp` opens afterward.
- Re-open the converted book and confirm the library shows the cached `.rsvp` rather than the pending EPUB.

## SD Card And Books

- Confirm `/books` exists on the SD card.
- Confirm `.rsvp.tmp`, `.rsvp.failed`, and `.rsvp.converting` files are removed after successful conversion.
- Test the desktop converter in `tools/sd_card_converter` on at least one EPUB.

## Repository

- Confirm `git status --short` only contains intentional changes.
- Run `git diff --check`.
- Confirm no sample copyrighted books, local `.epub` files, generated `.rsvp` files, or PlatformIO build outputs are staged.
- Choose and add a license before presenting the project as open source.
- Update `README.md` if hardware pins, build environments, or SD-card behavior changed.

## Suggested Push Flow

```sh
git status --short
git diff --check
pio run
pio run -e waveshare_esp32s3_usb_msc
git add .
git commit -m "Prepare RSVP reader public milestone"
git push origin HEAD
```
