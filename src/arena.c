#include "arena.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/*
* Returns the number of available bytes in the arena.
*/
static size_t get_open_space(const arena_t *arena) {
    if (!arena) {
        return 0;
    }
    return arena->capacity - arena->offset;
}


/*
* Resizes the memory block pointed at by `mem` to a size of `new_capacity`.
* It returns a pointer to the new memory block upong success and NULL upon
* failure. If the reallocation fails, note that the original memory block
* remains completely valid and unchanged.
*/
static char *resize_memory(char *mem, size_t new_capacity) {
    if (!mem || new_capacity > MAX_CAPACITY) {
        return NULL;
    }
    /* The C standard guarantees realloc returns an aligned address that is
    * suitable for all data types (i.e. realloc will returns an address aligned
    * to ALIGNMENT). */
    return (char *)realloc(mem, new_capacity);
}


/*
* Align `val` up to the closest aligned value according to `alignment`
*/
static size_t align_up(size_t val, size_t alignment) {
    return (val + alignment - 1) & ~(alignment -1);
}


arena_t *create_arena(size_t capacity) {
    if (!capacity || capacity > MAX_CAPACITY) {
        return NULL;
    }

    arena_t *arena = (arena_t *)malloc(sizeof(arena_t));
    if (!arena) {
        return NULL;
    }

    char *mem = (char *)malloc(capacity);
    if (!mem) {
        free(arena);
        return NULL;
    }

    memset(mem, 0, capacity);
    arena->start = mem;
    arena->capacity = capacity;
    arena->offset = 0;

    return arena;
}


void free_arena(arena_t *arena) {
    if (!arena) {
        return;
    }
    free(arena->start);
    free(arena);
}


size_t awrite(const char *src, size_t sz, size_t alignment, arena_t *arena) {
    if (!arena || !src || !sz || sz > MAX_CAPACITY || !alignment) {
        return SIZE_MAX;
    }

    /* alignment should be a power of 2 returned by _Alignof */
    if (!(alignment & (alignment - 1))) {
        return SIZE_MAX;
    }

    size_t aligned_offset = align_up(arena->offset, alignment);
    size_t space_required = aligned_offset - arena->offset + sz;  

    /* Resize if needed */
    if (get_open_space(arena) < space_required) {
        size_t new_capacity = align_up(2 * arena->capacity + sz, alignment);
        char *new_mem = resize_memory(arena->start, new_capacity);
        if (!new_mem) {
            return SIZE_MAX;
        }
        arena->start = new_mem;
        arena->capacity = new_capacity;
    }
    
    /* Write `sz` bytes at the closest aligned offset */
    memcpy(arena->start + aligned_offset, src, sz);
    arena->offset = aligned_offset + sz;

    return aligned_offset;
}

size_t set_char_at(char c, size_t target_offset, arena_t *arena) {
    if (!arena || !target_offset || target_offset > MAX_CAPACITY - 1) { 
        return SIZE_MAX;
    }

   char *target = arena->start + target_offset;
   *target = c;

    if (target_offset >= arena->offset) {
        arena->offset = target_offset + 1;
    }

    return 0;
}
