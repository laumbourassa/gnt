/*
 * gnt.h - Generic Nibble Trie interface
 * 
 * Copyright (c) 2024 Laurent Mailloux-Bourassa
 * 
 * This file is part of the Generic Nibble Trie (GNT) library.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GNT_H
#define GNT_H

#include <stdint.h>

#define	GNT_FORCE_INLINE inline __attribute__((always_inline))

typedef struct gnt_trie gnt_trie_t;
typedef uintptr_t gnt_key_t;
typedef uintptr_t gnt_data_t;
typedef int8_t gnt_status_t;
typedef uint8_t gnt_byte_t;
typedef size_t gnt_index_t;

typedef gnt_status_t (*gnt_accessor_t)(gnt_byte_t* byte, gnt_key_t key, gnt_index_t index);
typedef void (*gnt_deallocator_t)(gnt_data_t data);

typedef struct gnt_cfg
{
    gnt_accessor_t accessor;
    gnt_deallocator_t deallocator;
} gnt_cfg_t;

// Conversion functions for various types to gnt_data_t
static GNT_FORCE_INLINE gnt_data_t _gnt_int8_to_data(int8_t data) {return (gnt_data_t) data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_int16_to_data(int16_t data) {return (gnt_data_t) data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_int32_to_data(int32_t data) {return (gnt_data_t) data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_int64_to_data(int64_t data) {return (gnt_data_t) data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_uint8_to_data(uint8_t data) {return (gnt_data_t) data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_uint16_to_data(uint16_t data) {return (gnt_data_t) data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_uint32_to_data(uint32_t data) {return (gnt_data_t) data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_uint64_to_data(uint64_t data) {return (gnt_data_t) data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_float_to_data(float data) {return *(gnt_data_t*) &data;}
static GNT_FORCE_INLINE gnt_data_t _gnt_double_to_data(double data) {return *(gnt_data_t*) &data;}
//static GNT_FORCE_INLINE gnt_data_t _gnt_longdouble_to_data(long double data) {return *(gnt_data_t*) &data;} // Long double can be larger than gnt_data_t
static GNT_FORCE_INLINE gnt_data_t _gnt_voidptr_to_data(void* data) {return (gnt_data_t) data;}

#define GNT_DATA(data)  _Generic((data),        \
        int8_t: _gnt_int8_to_data,              \
        int16_t: _gnt_int16_to_data,            \
        int32_t: _gnt_int32_to_data,            \
        int64_t: _gnt_int64_to_data,            \
        uint8_t: _gnt_uint8_to_data,            \
        uint16_t: _gnt_uint16_to_data,          \
        uint32_t: _gnt_uint32_to_data,          \
        uint64_t: _gnt_uint64_to_data,          \
        float: _gnt_float_to_data,              \
        double: _gnt_double_to_data,            \
        default: _gnt_voidptr_to_data           \
        )(data)

#define GNT_KEY(key) GNT_DATA(key)

#define GNT_FLOAT(data)         (*(float*) &data)
#define GNT_DOUBLE(data)        (*(double*) &data)

/**
 * @brief Creates a new nibble trie.
 * 
 * @param cfg The trie configuration.
 * @return Pointer to the created gnt_trie_t or NULL on failure.
 */
gnt_trie_t* gnt_create(gnt_cfg_t* cfg);

/**
 * @brief Destroys the entire trie and frees all allocated memory.
 * 
 * @param trie The trie to destroy.
 * @return 0 on success, -1 on failure.
 */
gnt_status_t gnt_destroy(gnt_trie_t* trie);

/**
 * @brief Inserts data in the trie and associates it to a key.
 * 
 * @param trie The trie to insert the data into.
 * @param key The key to associate the data with.
 * @param data The data to insert.
 * @return 0 on success, -1 on failure.
 */
gnt_status_t gnt_insert(gnt_trie_t* trie, gnt_key_t key, gnt_data_t data);

/**
 * @brief Searches and returns the data associated to a key.
 * 
 * @param trie The trie to search.
 * @param key The key associated to the data.
 * @return The data or 0 if trie is empty or data isn't found.
 */
gnt_data_t gnt_search(gnt_trie_t* trie, gnt_key_t key);

/**
 * @brief Deletes the data associated to a key.
 * 
 * @param trie The trie to delete the data from.
 * @param key The key associated to the data.
 * @return 0 on success, -1 on failure.
 */
gnt_status_t gnt_delete(gnt_trie_t* trie, gnt_key_t key);

/**
 * @brief Returns the byte of the key at the index.
 * 
 * @param byte The returned byte.
 * @param key The key to get the byte from.
 * @param index The index to of the byte.
 * @return 0 on success, -1 on failure.
 */
gnt_status_t gnt_accessor_string(gnt_byte_t* byte, gnt_key_t key, gnt_index_t index);

#endif /* GNT_H */