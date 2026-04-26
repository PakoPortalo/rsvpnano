#include "display/axs15231b.h"
#include "platform/DesktopInput.h"
#include "board/BoardConfig.h"
#include "app/App.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

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

// ---- Vector text overlay ----
// DisplayManager pushes word draw requests here so we can re-render the central
// word with a real TTF on top of the bitmap layer (which would otherwise look
// pixelated when scaled).
struct OverlayWord {
    std::string text;
    int   x;          // logical x at which the bitmap left edge sits
    int   y;          // logical y of bitmap top edge
    int   pixelHeight;// approx glyph height in logical pixels (for ptsize)
    int   focusIndex; // index of the focus letter (red), -1 if none
    uint32_t baseColor;
    uint32_t focusColor;
};
static std::vector<OverlayWord> gOverlayQueue;
static TTF_Font* gTtfFontBig    = nullptr;  // sized for 70-px glyphs
static TTF_Font* gTtfFontMedium = nullptr;  // sized for ~36-px glyphs
static int       gTtfFontBigSize    = 0;
static int       gTtfFontMediumSize = 0;
static bool      gTtfReady      = false;

// ────────────────────────────────────────────────────────────────────────────
// TTF overlay tuning knobs — edit these to nudge the central word's look.
// All values are in *logical* pixels (640×172 space); they get multiplied by
// kScale automatically when rendered. Positive offsets push down / right.
// ────────────────────────────────────────────────────────────────────────────

// How tall the capital letters (e.g. "X") of the BIG central word should be.
// Increase → larger word. Bitmap font visual cap was ≈ 50.
static constexpr int kTtfBigCapsHeight    = 75;

// Same for the medium-size word (used when the smaller serif is active).
static constexpr int kTtfMediumCapsHeight = 28;

// Extra vertical nudge (in logical px). Positive = move text DOWN.
static constexpr int kTtfYOffset = 17;

// Extra horizontal nudge (in logical px). Positive = move text RIGHT.
// (Affects the focus letter's horizontal anchoring; usually leave at 0.)
static constexpr int kTtfXOffset = 0;

DesktopTouchEvent gPendingTouch;

static const char* findSerifFontPath() {
    if (const char* env = std::getenv("RSVP_FONT_PATH")) {
        return env;
    }
    static const char* candidates[] = {
        "/System/Library/Fonts/Supplemental/Georgia.ttf",
        "/System/Library/Fonts/NewYork.ttf",
        "/System/Library/Fonts/Supplemental/Times New Roman.ttf",
        "/Library/Fonts/Georgia.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        nullptr,
    };
    for (int i = 0; candidates[i]; ++i) {
        FILE* f = std::fopen(candidates[i], "rb");
        if (f) { std::fclose(f); return candidates[i]; }
    }
    return nullptr;
}

// Open font sized so that the cap-height of 'X' is approximately targetPxCaps.
static TTF_Font* openFontForCapsHeight(const char* path, int targetPxCaps) {
    int pt = targetPxCaps;
    TTF_Font* f = TTF_OpenFont(path, pt);
    if (!f) return nullptr;
    int minY = 0, maxY = 0;
    TTF_GlyphMetrics(f, 'X', nullptr, nullptr, &minY, &maxY, nullptr);
    int caps = maxY - minY;
    if (caps > 0 && caps != targetPxCaps) {
        const int newPt = std::max(8, pt * targetPxCaps / caps);
        TTF_CloseFont(f);
        f = TTF_OpenFont(path, newPt);
    }
    return f;
}

static void initTtf() {
    if (TTF_Init() != 0) {
        fprintf(stderr, "[ttf] TTF_Init failed: %s\n", TTF_GetError());
        return;
    }
    const char* path = findSerifFontPath();
    if (!path) {
        fprintf(stderr, "[ttf] No serif font found. Set RSVP_FONT_PATH to a .ttf path.\n");
        return;
    }
    // SIZE knob — see kTtfBigCapsHeight / kTtfMediumCapsHeight at top of file.
    gTtfFontBig = openFontForCapsHeight(path, kTtfBigCapsHeight * kScale);
    gTtfFontMedium = openFontForCapsHeight(path, kTtfMediumCapsHeight * kScale);
    if (!gTtfFontBig || !gTtfFontMedium) {
        fprintf(stderr, "[ttf] TTF_OpenFont failed for %s: %s\n", path, TTF_GetError());
        if (gTtfFontBig) { TTF_CloseFont(gTtfFontBig); gTtfFontBig = nullptr; }
        if (gTtfFontMedium) { TTF_CloseFont(gTtfFontMedium); gTtfFontMedium = nullptr; }
        return;
    }
    gTtfFontBigSize    = kTtfBigCapsHeight * kScale;
    gTtfFontMediumSize = kTtfMediumCapsHeight * kScale;
    gTtfReady = true;
    printf("[ttf] using %s (big-cap=%dpx, medium-cap=%dpx)\n", path,
           gTtfFontBigSize, gTtfFontMediumSize);
}

static void renderTtfStringSegment(TTF_Font* font, const char* text, int dstX, int dstY,
                                   uint32_t argb, int* outWidth) {
    if (!*text) { if (outWidth) *outWidth = 0; return; }
    SDL_Color col = { static_cast<uint8_t>((argb >> 16) & 0xFF),
                      static_cast<uint8_t>((argb >>  8) & 0xFF),
                      static_cast<uint8_t>((argb)       & 0xFF), 0xFF };
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, col);
    if (!surf) { if (outWidth) *outWidth = 0; return; }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(gRenderer, surf);
    if (tex) {
        SDL_Rect dst = { dstX, dstY, surf->w, surf->h };
        SDL_RenderCopy(gRenderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    if (outWidth) *outWidth = surf->w;
    SDL_FreeSurface(surf);
}

static int charAdvance(TTF_Font* font, char c) {
    int adv = 0;
    TTF_GlyphMetrics(font, static_cast<Uint16>(static_cast<unsigned char>(c)), nullptr, nullptr,
                     nullptr, nullptr, &adv);
    return adv;
}

static void renderOverlayQueue() {
    if (!gTtfReady || gOverlayQueue.empty()) {
        gOverlayQueue.clear();
        return;
    }
    for (const OverlayWord& w : gOverlayQueue) {
        TTF_Font* font = (w.pixelHeight > 50) ? gTtfFontBig : gTtfFontMedium;
        if (!font) continue;

        // w.x carries the anchor (focus letter centre, in logical px).
        // POSITION knobs: kTtfXOffset / kTtfYOffset shift the rendered word.
        const int anchorWinX = (w.x + kTtfXOffset) * kScale;
        // Bitmap glyph cell top → logical y; map to window and compensate for
        // TTF baseline so caps top sits at the bitmap glyph cell top.
        const int capsHeight = (w.pixelHeight > 50) ? gTtfFontBigSize : gTtfFontMediumSize;
        const int ascent = TTF_FontAscent(font);
        const int glyphCellTop = (w.y + kTtfYOffset) * kScale;
        // Surface y where caps top lands at glyphCellTop:
        const int surfaceY = glyphCellTop - (ascent - capsHeight);

        // Compute starting x so that focus letter centre lands at anchorWinX.
        int startX = anchorWinX;
        if (w.focusIndex >= 0 && w.focusIndex < static_cast<int>(w.text.size())) {
            int prefixW = 0;
            for (int i = 0; i < w.focusIndex; ++i) {
                prefixW += charAdvance(font, w.text[i]);
            }
            const int focusW = charAdvance(font, w.text[w.focusIndex]);
            startX = anchorWinX - prefixW - focusW / 2;
        } else {
            int totalW = 0;
            TTF_SizeUTF8(font, w.text.c_str(), &totalW, nullptr);
            startX = anchorWinX - totalW / 2;
        }

        int cursorX = startX;
        for (size_t i = 0; i < w.text.size(); ++i) {
            char buf[2] = { w.text[i], 0 };
            const uint32_t col = (static_cast<int>(i) == w.focusIndex) ? w.focusColor : w.baseColor;
            int charW = 0;
            renderTtfStringSegment(font, buf, cursorX, surfaceY, col, &charW);
            cursorX += charAdvance(font, w.text[i]);
            (void)charW;
        }
    }
    gOverlayQueue.clear();
}

extern "C" void desktopQueueWord(const char* text, int x, int y, int pixelHeight, int focusIndex,
                                 uint32_t baseRgb, uint32_t focusRgb) {
    if (!gTtfReady || !text) return;
    OverlayWord w;
    w.text = text;
    w.x = x;
    w.y = y;
    w.pixelHeight = pixelHeight;
    w.focusIndex = focusIndex;
    w.baseColor = baseRgb;
    w.focusColor = focusRgb;
    gOverlayQueue.push_back(std::move(w));
}

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
    initTtf();
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
        renderOverlayQueue();
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
                        app.desktopAction(App::DesktopAction::UpPress, shift ? 10 : 1);
                        break;
                    case SDLK_DOWN:
                        app.desktopAction(App::DesktopAction::DownPress, shift ? 10 : 1);
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
                    case SDLK_UP:
                        app.desktopAction(App::DesktopAction::UpRelease);
                        break;
                    case SDLK_DOWN:
                        app.desktopAction(App::DesktopAction::DownRelease);
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
