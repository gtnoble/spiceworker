
#ifndef BZY_ARENA
#define BZY_ARENA

#include <stdlib.h>
#include <stdalign.h>

#define ARENA_ALLOCATE(length, type, arena) \
    (type *) arena_allocate_aligned(length * sizeof(type), alignof(type), arena)

#define ARENA_PUSH(object, arena) \
    (typeof(object) *) arena_push(&object, sizeof(object), alignof(typeof(object)), arena)

typedef struct {
    char *memory;
    size_t offset;
    size_t size;
} Arena;

Arena make_arena(size_t max_size);
void free_arena(Arena *arena);
void *arena_allocate(size_t size, Arena *arena);
void *arena_allocate_aligned(size_t size, size_t alignment, Arena *arena);
void *arena_push(void *object, size_t size, size_t alignment, Arena *arena);
void arena_reset(Arena *arena);

size_t arena_checkpoint(const Arena *arena);
void arena_restore(size_t checkpoint, Arena *arena);

void *arena_allocate_zeros(size_t size, Arena *arena);

char *arena_duplicate_string(const char *string, Arena *arena);
char *arena_sprintf(Arena *arena, const char *format, ...);
char *arena_sprintf_null(Arena *arena, const char *format, ...);
char *arena_sprint_null(Arena *arena);
char *arena_strdup(Arena *arena, const char *string);
char *arena_strndup(Arena *arena, const char *string, size_t max_length);

#endif
