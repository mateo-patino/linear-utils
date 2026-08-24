#ifndef ARENA_H
#define ARENA_H


/*
* arena.h and arena.c provide an interface for a memory arena. Memory
* arenas are wrapped around a arena_t struct, which contains three
* members: `start` which is a pointer to the first byte in the arena,
* `capacity` which is the number of bytes in the arena, and `offset`
* which is a pointer to the address where the next byte would be
* written.
*
* Callers to this interface should not modify arena_t. This struct is
* meant to be used exclusively by the functions in this module. Instead,
* callers must interact with this library via the public functions 
* provided.
*
* The arena uses type-specific alignments when allocating objects. Use _Alignof(T)
* to get the alignment for a specific type and pass it to the allocator functions.
*/

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>


#define KiB(n) ((n) * (1ULL << 10))
#define MiB(n) ((n) * (1ULL << 20))
#define GiB(n) ((n) * (1ULL << 30))


/* The / 4 is a safety margin since various expressions in this interface
* can lead to overflow (e.g. val + ALIGNMENT, current_offset + sz, etc.).
* We'll simply reject any arenas greater than SIZE_MAX / 4 as well as any
*  `sz` arguments in the allocator that are greater than this value. */
#define MAX_CAPACITY (SIZE_MAX / 4)

typedef struct { 
    char *start;
    size_t capacity;
    size_t offset;
} arena_t;


/*
* Creates an arena of `capacity` bytes and returns a pointer to an arena_t
* object where `arena_t.start` will point to the first byte in the arena upon
* success.
*
* The arena_t struct pointed at by the returned pointer lives in the heap.
* Calling `free_arena` will free the arena's memory and the arena_t struct.
*
* NULL is returned upon failure.
*/
arena_t *create_arena(size_t capacity);


/*
* Frees a memory arena and its parent arena_t struct 
*/
void free_arena(arena_t *arena);


/*
* Writes `sz` bytes from the address `src` to `arena`. In other words,
* this function allocates an object of `sz` bytes pointer at by `src`
* to the memory arena owned by the `arena` struct.
*
* If the arena cannot fit the new object, the arena gets resized 
* automatically.
*
* It returns an integer representing the offset from `arena->start` where the
* object was allocated. Upon failure, SIZE_MAX is returned. It is
* guaranteed that SIZE_MAX will never be a valid offset value given the
* MAX_CAPACITY constraint.
*/
size_t awrite(const char *src, size_t sz, size_t alignment, arena_t *arena);


/*
* Write a char `c` at an specific offset `target_offset`.
* 
* If `target_offset` is smaller than `arena->offset`, i.e., it is an offset at a 
* previously allocated byte, the old value will be overwritten. `arena->offset` is 
* not moved down in this case. If `target_offset` is larger than or equal to `arena->offset` 
* the new value is written and `arena->offset` is advanced to `target_offset + 1` so that
* future writes to the arena do not overwrite the new value.
*
* If `target_offset` is larger than the capacity of the arena, SIZE_MAX is returned and 
* no value is written. This function does not resize the arena.
*
* 0 is returned upon success and SIZE_MAX is returned if the character could not be written.
*/
size_t set_char_at(char c, size_t target_offset, arena_t *arena);




#endif
