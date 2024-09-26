#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "sand.h"

Arena make_arena(size_t max_size) {
    Arena arena;
    arena.memory = mmap(
        NULL, 
        max_size, 
        PROT_READ | PROT_WRITE, 
        MAP_SHARED | MAP_ANONYMOUS, 
        -1, 
        0
    );
    arena.offset = 0;
    arena.size = max_size; 
    return arena;
}

void free_arena(Arena *arena) {
    munmap(arena->memory, arena->size);
}

void *arena_allocate(size_t size, Arena *arena) {
    if (arena->offset + size > arena->size - 1) {
        fprintf(
            stderr, 
            "error: Out of memory, cannot allocate from arena: "
            "arena size: %zu: "
            "memory remaining: %zu: "
            "requested: %zu", 
            arena->size,
            arena->size - arena->offset,
            size
        );
        exit(1);
    }
    void *allocated_memory = &arena->memory[arena->offset];
    arena->offset += size;
    return allocated_memory;
}

void *arena_allocate_aligned(size_t size, size_t alignment, Arena *arena) {
    arena->offset += alignment - ((arena->offset % alignment) % alignment);
    return arena_allocate(size, arena);
}

void* arena_push(void *object, size_t size, size_t alignment, Arena *arena) {
    void *allocation = arena_allocate_aligned(size, alignment, arena);
    memcpy(allocation, object, size);
    return allocation;
}


void *arena_allocate_zeros(size_t size, Arena *arena) {
    char *allocated_memory = &arena->memory[arena->offset];
    for (size_t i = 0; i < size; i++) {
        allocated_memory[arena->offset + i] = 0;
    }
    arena->offset += size;
    return (void *) allocated_memory;
}

char *arena_duplicate_string(const char *string, Arena *arena) {
    size_t length = strlen(string);
    char *duplicated_string = arena_allocate(sizeof(char) * (length + 1), arena);
    strcpy(duplicated_string, string);
    return duplicated_string;
}

char *arena_vsprintf(Arena *arena, const char *format, va_list argptr) {

    char *printed_string = arena->memory + arena->offset;

    int num_chars_printed = vsprintf(printed_string, format, argptr);
    assert(num_chars_printed >= 0);

    arena->offset += num_chars_printed;

    return printed_string;
    
}

char *arena_sprintf(Arena *arena, const char *format, ...) {

    va_list argptr;
    va_start(argptr, format);

    char *string = arena_vsprintf(arena, format, argptr);
    va_end(argptr);
    return string;
}

char *arena_sprintf_null(Arena *arena, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);

    char *string = arena_vsprintf(arena, format, argptr);
    va_end(argptr);
    arena_sprint_null(arena);
    return string;
}

char *arena_sprint_null(Arena *arena) {
    char *null_char = ARENA_ALLOCATE(1, char, arena);
    *null_char = '\0';
    return null_char;
}

char *arena_strdup(Arena *arena, const char *string) {
    char *copy = ARENA_ALLOCATE(strlen(string) + 1, char, arena);
    strcpy(copy, string);
    return copy;
}

char *arena_strndup(Arena *arena, const char *string, size_t max_length) {
    size_t length = strnlen(string, max_length);

    char *copy = ARENA_ALLOCATE(length + 1, char, arena);
    strncpy(copy, string, length);
    copy[length + 1] = '\0';
    return copy;
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
}

size_t arena_checkpoint(const Arena *arena) {
    return arena->offset;
}

void arena_restore(size_t checkpoint, Arena *arena) {
    arena->offset = checkpoint;
}


