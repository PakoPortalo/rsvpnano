#include "display/axs15231b.h"
#include "platform/DesktopInput.h"
#include "board/BoardConfig.h"
#include "app/App.h"

#include <SDL.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern App app;

#ifdef __APPLE__
#include <objc/runtime.h>
#include <objc/message.h>
// Tell macOS this is a foreground GUI app — required when launching from terminal.
static void macosActivateApp() {
    id app = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend)(
        reinterpret_cast<id>(objc_getClass("NSApplication")),
        sel_getUid("sharedApplication"));
    // NSApplicationActivationPolicyRegular = 0
    reinterpret_cast<void (*)(id, SEL, long)>(objc_msgSend)(
        app, sel_getUid("setActivationPolicy:"), 0L);
    reinterpret_cast<void (*)(id, SEL, bool)>(objc_msgSend)(
        app, sel_getUid("activateIgnoringOtherApps:"), true);
}
#endif

// Scale factor for window. 2 → 1280×344. Bitmap font is fixed-res so any
// scaling will pixelate; linear filter (set in axs15231bInit) softens it.
static constexpr int kScale = 2;
static constexpr int kLogW  = 640;   // logical display width
static constexpr int kLogH  = 172;   // logical display height

static SDL_Window*   gWindow   = nullptr;
static SDL_Renderer* gRenderer = nullptr;
static SDL_Texture*  gTexture  = nullptr;
static uint32_t      gPixels[kLogW * kLogH] = {};

DesktopTouchEvent gPendingTouch;

// Forward declaration (defined in Arduino.cpp extern linkage)
// nothing extra needed — desktopPumpEvents is defined here

void axs15231bInit() {
#ifdef __APPLE__
    macosActivateApp();
#endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "[display] SDL_Init failed: %s\n", SDL_GetError());
        return;
    }
    // Nearest-neighbour: crisp scaled pixels, mirrors the device look.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    gWindow = SDL_CreateWindow(
        "rsvpnano",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kLogW * kScale, kLogH * kScale,
        SDL_WINDOW_SHOWN);
    if (!gWindow) {
        fprintf(stderr, "[display] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return;
    }
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!gRenderer) {
        gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_SOFTWARE);
    }
    gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING, kLogW, kLogH);
    memset(gPixels, 0, sizeof(gPixels));
    SDL_ShowWindow(gWindow);
    SDL_RaiseWindow(gWindow);
    SDL_SetWindowInputFocus(gWindow);
    SDL_PumpEvents();
    printf("[display] window %dx%d (scale %dx from %dx%d)\n",
           kLogW * kScale, kLogH * kScale, kScale, kLogW, kLogH);
    printf("[display] Controls:\n");
    printf("  Space            play / pause (in menu: select)\n");
    printf("  Enter / M        open / close menu\n");
    printf("  Esc              back (pause if playing, exit menu)\n");
    printf("  Left / Right     hold to scrub at WPM rate (in menu: back / enter)\n");
    printf("  Shift+Left/Right scrub -10 / +10 words\n");
    printf("  Up / Down        WPM +1 / -1               (Shift = x10)\n");
    printf("                   (in menu: navigate up / down)\n");
    printf("  T                cycle theme (light / dark / night)\n");
    printf("  Mouse click      tap (use to wake splash)\n");
    printf("  Q                quit\n");
}

void axs15231bSetBacklight(bool) {}
void axs15231bSetBrightnessPercent(uint8_t) {}
void axs15231bSleep() {}
void axs15231bWake() {}

// Receives panel-native chunks (portrait, 172 cols × 640 rows).
// With UI_ROTATED_180=true in flushScaledFrame:
//   nativeX = panel column (0..171), nativeY = panel row (0..639)
//   logicalX = nativeY,  logicalY = 171 - nativeX
// We undo that to get logical (640×172) texture coordinates.
void axs15231bPushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                         const uint16_t* data) {
    if (!gTexture || !data || width == 0 || height == 0) return;

    for (uint16_t localNativeY = 0; localNativeY < height; ++localNativeY) {
        for (uint16_t nativeX = 0; nativeX < width; ++nativeX) {
            const uint16_t pnX = nativeX;
            const uint16_t pnY = static_cast<uint16_t>(y + localNativeY);

            // Invert the rotation from flushScaledFrame (UI_ROTATED_180=true):
            //   logX = pnY  (0..639)
            //   logY = (kLogH-1) - pnX  (0..171)
            const int logX = pnY;
            const int logY = (kLogH - 1) - pnX;

            if (logX < 0 || logX >= kLogW || logY < 0 || logY >= kLogH) continue;

            // Pixels from DisplayManager are byte-swapped for the SPI panel.
            // Un-swap to get native RGB565.
            const uint16_t swapped = data[localNativeY * width + nativeX];
            const uint16_t rgb565  = static_cast<uint16_t>((swapped << 8) | (swapped >> 8));

            const uint8_t r = static_cast<uint8_t>(((rgb565 >> 11) & 0x1F) * 255 / 31);
            const uint8_t g = static_cast<uint8_t>(((rgb565 >>  5) & 0x3F) * 255 / 63);
            const uint8_t b = static_cast<uint8_t>(((rgb565      ) & 0x1F) * 255 / 31);

            gPixels[logY * kLogW + logX] = (0xFF000000u | (static_cast<uint32_t>(r) << 16)
                                                         | (static_cast<uint32_t>(g) << 8)
                                                         | static_cast<uint32_t>(b));
        }
    }

    // Only present at the end of a full frame (last chunk covers to bottom of panel)
    if (static_cast<int>(y + height) >= BoardConfig::PANEL_NATIVE_HEIGHT) {
        SDL_UpdateTexture(gTexture, nullptr, gPixels, kLogW * sizeof(uint32_t));
        SDL_RenderClear(gRenderer);
        SDL_RenderCopy(gRenderer, gTexture, nullptr, nullptr);
        SDL_RenderPresent(gRenderer);
    }
}

// ---- Event pump — called from delay() ----

static void mouseTap(int px, int py) {
    const uint16_t logX = static_cast<uint16_t>(px / kScale);
    const uint16_t logY = static_cast<uint16_t>(py / kScale);
    gPendingTouch = { true, true, logX, logY, 0 };
}

static void mouseTapEnd(int px, int py) {
    const uint16_t logX = static_cast<uint16_t>(px / kScale);
    const uint16_t logY = static_cast<uint16_t>(py / kScale);
    gPendingTouch = { true, false, logX, logY, 2 };
}

void desktopPumpEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
                break;

            case SDL_KEYDOWN: {
                if (event.key.repeat) break;
                const bool shift = (SDL_GetModState() & KMOD_SHIFT) != 0;
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        exit(0);
                        break;
                    case SDLK_ESCAPE:
                        app.desktopAction(App::DesktopAction::Back);
                        break;
                    case SDLK_SPACE:
                        gDesktopPinState[0] = 0;  // BOOT pressed (Paused/Playing toggle)
                        app.desktopAction(App::DesktopAction::Select);  // Menu select
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                    case SDLK_m:
                        gDesktopPinState[16] = 0;  // PWR pressed (open/close menu)
                        break;
                    case SDLK_LEFT:
                        app.desktopAction(shift ? App::DesktopAction::LeftFast
                                                : App::DesktopAction::LeftPress);
                        break;
                    case SDLK_RIGHT:
                        app.desktopAction(shift ? App::DesktopAction::RightFast
                                                : App::DesktopAction::RightPress);
                        break;
                    case SDLK_UP:
                        app.desktopAction(shift ? App::DesktopAction::UpFast
                                                : App::DesktopAction::Up);
                        break;
                    case SDLK_DOWN:
                        app.desktopAction(shift ? App::DesktopAction::DownFast
                                                : App::DesktopAction::Down);
                        break;
                    case SDLK_t:
                        app.desktopAction(App::DesktopAction::ThemeCycle);
                        break;
                    default:
                        break;
                }
                break;
            }

            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        gDesktopPinState[0] = 1;
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                    case SDLK_m:
                        gDesktopPinState[16] = 1;
                        break;
                    case SDLK_LEFT:
                        app.desktopAction(App::DesktopAction::LeftRelease);
                        break;
                    case SDLK_RIGHT:
                        app.desktopAction(App::DesktopAction::RightRelease);
                        break;
                    default:
                        break;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseTap(event.button.x, event.button.y);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseTapEnd(event.button.x, event.button.y);
                }
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_SHOWN:
                    case SDL_WINDOWEVENT_RESTORED:
                        SDL_RaiseWindow(gWindow);
                        SDL_SetWindowInputFocus(gWindow);
                        break;
                    default:
                        break;
                }
                break;

            case SDL_APP_WILLENTERFOREGROUND:
            case SDL_APP_DIDENTERFOREGROUND:
                if (gWindow) {
                    SDL_ShowWindow(gWindow);
                    SDL_RaiseWindow(gWindow);
                    SDL_SetWindowInputFocus(gWindow);
                }
                break;

            default:
                break;
        }
    }
}
