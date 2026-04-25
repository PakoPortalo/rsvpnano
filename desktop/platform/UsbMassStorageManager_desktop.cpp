#include "usb/UsbMassStorageManager.h"

// USB transfer fully stubbed on desktop (RSVP_USB_TRANSFER_ENABLED=0).

UsbMassStorageManager* UsbMassStorageManager::instance_ = nullptr;

UsbMassStorageManager::UsbMassStorageManager() {}

bool UsbMassStorageManager::begin(bool) { return false; }
void UsbMassStorageManager::end() {}
bool UsbMassStorageManager::active() const { return false; }
bool UsbMassStorageManager::ejected() const { return false; }
bool UsbMassStorageManager::writeEnabled() const { return false; }
uint64_t UsbMassStorageManager::cardSizeBytes() const { return 0; }
const char* UsbMassStorageManager::statusMessage() const { return "USB disabled"; }

bool UsbMassStorageManager::beginSdCard() { return false; }
void UsbMassStorageManager::endSdCard() {}
bool UsbMassStorageManager::configureMsc() { return false; }

int32_t UsbMassStorageManager::readSectors(uint32_t, uint32_t, void*, uint32_t) { return -1; }
int32_t UsbMassStorageManager::writeSectors(uint32_t, uint32_t, uint8_t*, uint32_t) { return -1; }
bool UsbMassStorageManager::handleStartStop(uint8_t, bool, bool) { return true; }

int32_t UsbMassStorageManager::onRead(uint32_t, uint32_t, void*, uint32_t) { return -1; }
int32_t UsbMassStorageManager::onWrite(uint32_t, uint32_t, uint8_t*, uint32_t) { return -1; }
bool UsbMassStorageManager::onStartStop(uint8_t, bool, bool) { return true; }
