# rsvpnano — desktop port

Fork of [ionutdecebal/rsvpnano](https://github.com/ionutdecebal/rsvpnano) focused on porting the RSVP reader to desktop and mobile platforms while keeping the original ESP32-S3 firmware buildable.

## Status

| Platform | State |
|----------|-------|
| macOS (Apple Silicon, M1 / M2) | ✅ Working — SDL2 simulator under `desktop/` |
| Windows | ⏳ Planned (next) |
| Linux | ⏳ Planned |
| Android | 🎯 Roadmap |
| iOS | 🎯 Roadmap |
| ESP32-S3 device (original) | ✅ Still builds with `pio run` |

The macOS build runs the same `src/` code as the device. Hardware (display, buttons, touch, SD, USB) is replaced by a thin shim layer (`desktop/shim/`, `desktop/platform/`) that bridges to SDL2 and POSIX.

## Quick start (macOS Apple Silicon)

```bash
brew install sdl2
cd desktop
make
./build/rsvpnano
```

The window opens at 1280×344 (logical 640×172 ×2). Drop `.txt` or `.rsvp` books in `desktop/books/` and they appear in the library on next launch. EPUB conversion is disabled in the desktop build for now — convert on the device or with another tool first.

Click inside the window once on launch so it captures keyboard focus.

## Keyboard controls (desktop)

| Key | Reader (Paused / Playing) | Menu |
|-----|---------------------------|------|
| **Space** | Play / Pause | Select item |
| **Enter** / **M** | Open menu | Close menu |
| **Esc** | Pause if playing | Back to parent menu |
| **← / →** | Hold to scrub at the current WPM | Back / Enter |
| **Shift+← / Shift+→** | Jump ±10 words | — |
| **↑ / ↓** | WPM ±1 | Navigate up / down |
| **Shift+↑ / Shift+↓** | WPM ±10 | — |
| **T** | Cycle theme (light / dark / night) | — |
| **Q** | Quit | Quit |
| Mouse left click | Tap (wakes the splash) | Tap = select |

Holding ← / → advances words at the current reading speed — natural for seeking inside a paragraph without spamming the key.

## Build the device firmware

The original ESP32-S3 firmware still builds:

```bash
pio run -e waveshare_esp32s3_usb_msc
pio run -t upload -e waveshare_esp32s3_usb_msc
```

Hardware target is the Waveshare ESP32-S3-Touch-LCD-3.49 (640×172 landscape). See the upstream repo for the full hardware story, web flasher, and the on-device EPUB conversion pipeline: <https://github.com/ionutdecebal/rsvpnano>.

## Repo layout

```
src/                     Cross-platform reader code (unchanged on device)
desktop/
  Makefile               clang++ build for macOS
  main_desktop.cpp       SDL entry point, drives setup() / loop()
  platform/              Hardware replacements (display → SDL2, input → keyboard, ...)
  shim/                  ESP-IDF / Arduino API stubs for desktop
  books/                 Drop .txt or .rsvp books here
```

`#if DESKTOP_BUILD` guards the few desktop-only hooks added inside `src/app/App.{h,cpp}` (the keyboard-driven action dispatcher). Device builds are unaffected.

## Roadmap

1. **macOS** — currently working. Known issue: the bitmap font is fixed-resolution, so any window scaling either pixelates (nearest filter) or blurs (linear). The proper fix is vector text on desktop via SDL_ttf.
2. **Windows** — replace the Makefile with CMake or MSVC, verify SDL2 specifics, drop the macOS-only NSApplication activation in `axs15231b_desktop.cpp`.
3. **Linux** — should follow Windows for free given SDL2's portability; mostly packaging.
4. **Android** — wrap the SDL2 layer with SDL2's Android template, swap SD-card storage for the platform's storage API, ship as an APK.
5. **iOS** — same story with SDL2's iOS template; rework storage and entry point; package as an `.ipa`.

End goal: a single codebase where `src/` stays the reader logic and each platform contributes its own thin shim layer.

## License

Inherits the upstream repo's license. See `LICENSE` at the upstream repository.
