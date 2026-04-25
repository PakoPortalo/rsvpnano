#pragma once
#include <cstdlib>
#include <cstddef>
#include <cstdint>

#define MALLOC_CAP_EXEC     (1 << 0)
#define MALLOC_CAP_32BIT    (1 << 1)
#define MALLOC_CAP_8BIT     (1 << 2)
#define MALLOC_CAP_DMA      (1 << 3)
#define MALLOC_CAP_SPIRAM   (1 << 4)
#define MALLOC_CAP_INTERNAL (1 << 5)
#define MALLOC_CAP_DEFAULT  (1 << 6)

inline void* heap_caps_malloc(size_t size, uint32_t) { return malloc(size); }
inline void* heap_caps_realloc(void* ptr, size_t size, uint32_t) { return realloc(ptr, size); }
inline void  heap_caps_free(void* ptr) { free(ptr); }
inline size_t heap_caps_get_free_size(uint32_t) { return 32UL * 1024UL * 1024UL; }
inline size_t heap_caps_get_largest_free_block(uint32_t) { return 16UL * 1024UL * 1024UL; }
inline size_t heap_caps_get_minimum_free_size(uint32_t) { return 8UL * 1024UL * 1024UL; }
