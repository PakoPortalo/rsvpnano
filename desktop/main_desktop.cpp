// Desktop entry point — calls Arduino-style setup()/loop() from src/main.cpp
#include <cstdio>
#include <thread>
#include <chrono>
#include <SDL_main.h>

extern void setup();
extern void loop();
extern void desktopPumpEvents();

int main(int /*argc*/, char** /*argv*/) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    setup();
    while (true) {
        desktopPumpEvents();
        loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}
