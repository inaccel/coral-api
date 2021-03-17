/**
 * Copyright © 2018-2021 InAccel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SHM_H
#define SHM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Allocate InAccel memory block
 *
 *  @details Allocates a block of size bytes of memory, returning a pointer to
 *           the beginning of the block. The content of the newly allocated
 *           block of memory is zero initialized. If size is zero the returned
 *           pointer shall not be dereferenced.
 *
 *  @param size Size of the memory block, in bytes. size_t is an unsigned
 *              integral type.
 *  @returns On success, a pointer to the memory block allocated by the
 *           function. The type of this pointer is always void*, which can be
 *           cast to the desired type of data pointer in order to be
 *           dereferenceable. If the function failed to allocate the requested
 *           block of memory, a null pointer is returned.
 */
void *inaccel_alloc(size_t size);

/** @brief Deallocate InAccel memory block
 *
 *  @details A block of memory previously allocated by a call to inaccel_alloc
 *           or inaccel_realloc is deallocated, making it available again for
 *           further allocations. If ptr does not point to a block of memory
 *           allocated with the above functions, it causes undefined behavior.
 *           If ptr is a null pointer, the function does nothing. Notice that
 *           this function does not change the value of ptr itself, hence it
 *           still points to the same (now invalid) location.
 *
 *  @param ptr Pointer to a memory block previously allocated with inaccel_alloc
 *             or inaccel_realloc.
 */
void inaccel_free(void *ptr);

/** @brief Reallocate InAccel memory block
 *
 *  @details Changes the size of the memory block pointed to by ptr. The
 *           function may move the memory block to a new location (whose address
 *           is returned by the function). The content of the memory block is
 *           preserved up to the lesser of the new and old sizes, even if the
 *           block is moved to a new location. If the new size is larger, the
 *           value of the newly allocated portion is zero. In case that ptr is
 *           a null pointer, the function behaves like inaccel_alloc, assigning
 *           a new block of size bytes and returning a pointer to its beginning.
 *
 *  @param ptr Pointer to a memory block previously allocated with inaccel_alloc
 *             or inaccel_realloc. Alternatively, this can be a null pointer, in
 *             which case a new block is allocated (as if inaccel_alloc was
 *             called).
 *  @param size New size for the memory block, in bytes. size_t is an unsigned
 *              integral type.
 *  @returns A pointer to the reallocated memory block, which may be either the
 *           same as ptr or a new location. The type of this pointer is void*,
 *           which can be cast to the desired type of data pointer in order to
 *           be dereferenceable.
 */
void *inaccel_realloc(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif // SHM_H
