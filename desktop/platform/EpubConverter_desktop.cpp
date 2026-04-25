#include "storage/EpubConverter.h"

// EPUB conversion disabled on desktop (RSVP_ON_DEVICE_EPUB_CONVERSION=0).
// These stubs satisfy the linker; the code paths are never reached.

bool EpubConverter::convertIfNeeded(const String&, const String&, const Options&) {
    return false;
}

bool EpubConverter::isCurrentCache(const String&) {
    return false;
}
