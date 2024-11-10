/*
 * gbt.h - Generic Bitwise Trie interface
 * 
 * Copyright (c) 2024 Laurent Mailloux-Bourassa
 * 
 * This file is part of the Generic Bitwise Trie (GBT) library.
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

#ifndef GBT_H
#define GBT_H

#include <stdint.h>

#define	GBT_FORCE_INLINE inline __attribute__((always_inline))

typedef struct gbt_trie gbt_trie_t;
typedef uintptr_t gbt_key_t;
typedef uintptr_t gbt_data_t;
typedef int8_t gbt_status_t;
typedef uint8_t gbt_byte_t;
typedef size_t gbt_index_t;

typedef gbt_status_t (*gbt_accessor_t)(gbt_byte_t* byte, gbt_key_t key, gbt_index_t index);
typedef void (*gbt_deallocator_t)(gbt_data_t data);

typedef struct gbt_cfg
{
    gbt_accessor_t accessor;
    gbt_deallocator_t deallocator;
} gbt_cfg_t;

// Conversion functions for various types to gbt_data_t
static GBT_FORCE_INLINE gbt_data_t _gbt_int8_to_data(int8_t data) {return (gbt_data_t) data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_int16_to_data(int16_t data) {return (gbt_data_t) data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_int32_to_data(int32_t data) {return (gbt_data_t) data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_int64_to_data(int64_t data) {return (gbt_data_t) data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_uint8_to_data(uint8_t data) {return (gbt_data_t) data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_uint16_to_data(uint16_t data) {return (gbt_data_t) data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_uint32_to_data(uint32_t data) {return (gbt_data_t) data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_uint64_to_data(uint64_t data) {return (gbt_data_t) data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_float_to_data(float data) {return *(gbt_data_t*) &data;}
static GBT_FORCE_INLINE gbt_data_t _gbt_double_to_data(double data) {return *(gbt_data_t*) &data;}
//static GBT_FORCE_INLINE gbt_data_t _gbt_longdouble_to_data(long double data) {return *(gbt_data_t*) &data;} // Long double can be larger than gbt_data_t
static GBT_FORCE_INLINE gbt_data_t _gbt_voidptr_to_data(void* data) {return (gbt_data_t) data;}

#define GBT_DATA(data)  _Generic((data),        \
        int8_t: _gbt_int8_to_data,              \
        int16_t: _gbt_int16_to_data,            \
        int32_t: _gbt_int32_to_data,            \
        int64_t: _gbt_int64_to_data,            \
        uint8_t: _gbt_uint8_to_data,            \
        uint16_t: _gbt_uint16_to_data,          \
        uint32_t: _gbt_uint32_to_data,          \
        uint64_t: _gbt_uint64_to_data,          \
        float: _gbt_float_to_data,              \
        double: _gbt_double_to_data,            \
        default: _gbt_voidptr_to_data           \
        )(data)

#define GBT_KEY(key) GBT_DATA(key)

#define GBT_FLOAT(data)         (*(float*) &data)
#define GBT_DOUBLE(data)        (*(double*) &data)

gbt_trie_t* gbt_create(gbt_cfg_t* cfg);
gbt_status_t gbt_destroy(gbt_trie_t* trie);
gbt_status_t gbt_insert(gbt_trie_t* trie, gbt_key_t key, gbt_data_t data);
gbt_data_t gbt_search(gbt_trie_t* trie, gbt_key_t key);
gbt_status_t gbt_delete(gbt_trie_t* trie, gbt_key_t key);
gbt_status_t gbt_accessor_string(gbt_byte_t* byte, gbt_key_t key, gbt_index_t index);

#endif /* GBT_H */