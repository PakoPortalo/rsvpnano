CXX      = clang++
CXXFLAGS = -std=c++17 -O0 -g \
           -Wall -Wextra \
           -Wno-unused-parameter -Wno-unused-variable \
           -Wno-missing-field-initializers \
           -Wno-ambiguous-macro

# SDL2 flags
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS   := -lSDL2main $(shell sdl2-config --libs) -framework AppKit -framework Foundation

DESKTOP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SHIM_DIR    := $(DESKTOP_DIR)shim
SRC_DIR     := $(DESKTOP_DIR)../src

DEFINES = \
  -DDESKTOP_BUILD=1 \
  -DRSVP_ON_DEVICE_EPUB_CONVERSION=0 \
  -DRSVP_USB_TRANSFER_ENABLED=0 \
  -DRSVP_MAX_BOOK_WORDS=0 \
  -DCORE_DEBUG_LEVEL=0

# Include order: shim FIRST to shadow ESP32 headers
INCLUDES = \
  -I$(SHIM_DIR) \
  -I$(DESKTOP_DIR) \
  -I$(SRC_DIR)

SOURCES = \
  $(DESKTOP_DIR)main_desktop.cpp \
  $(SRC_DIR)/main.cpp \
  $(SRC_DIR)/app/App.cpp \
  $(SRC_DIR)/display/DisplayManager.cpp \
  $(SRC_DIR)/reader/ReadingLoop.cpp \
  $(SRC_DIR)/storage/StorageManager.cpp \
  $(DESKTOP_DIR)platform/axs15231b_desktop.cpp \
  $(DESKTOP_DIR)platform/BoardConfig_desktop.cpp \
  $(DESKTOP_DIR)platform/ButtonHandler_desktop.cpp \
  $(DESKTOP_DIR)platform/TouchHandler_desktop.cpp \
  $(DESKTOP_DIR)platform/EpubConverter_desktop.cpp \
  $(DESKTOP_DIR)platform/UsbMassStorageManager_desktop.cpp \
  $(DESKTOP_DIR)platform/Wire_desktop.cpp \
  $(SHIM_DIR)/Arduino.cpp \
  $(SHIM_DIR)/FS.cpp \
  $(SHIM_DIR)/SD_MMC.cpp \
  $(SHIM_DIR)/Preferences.cpp

BUILD_DIR := $(DESKTOP_DIR)build
OBJECTS   := $(patsubst %,$(BUILD_DIR)/%.o,$(SOURCES))
TARGET    := $(BUILD_DIR)/rsvpnano

.PHONY: all clean run

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: % | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(SDL_CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(SDL_LIBS) -o $@
	@echo ""
	@echo "Built: $@"
	@echo "Place .txt or .rsvp books in: $(DESKTOP_DIR)books/"
	@echo "Run:   cd $(DESKTOP_DIR) && ./build/rsvpnano"

run: $(TARGET)
	cd $(DESKTOP_DIR) && ./build/rsvpnano

clean:
	rm -rf $(BUILD_DIR)
