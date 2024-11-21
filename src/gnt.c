/*
 * gnt.c - Generic Nibble Trie implementation
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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <threads.h>
#include "gnt.h"

#define GNT_HIGH_NIBBLE(byte)           (byte >> 4)
#define GNT_LOW_NIBBLE(byte)            (byte & 0x0F)
#define GNT_SELECT_KEY_BYTE(key, index) (key >> (8 * index))

typedef struct gnt_node gnt_node_t;

typedef struct gnt_nibble gnt_nibble_t;
typedef struct gnt_nibble // Represents the first 4 bits of a byte
{
    uint8_t children;
    gnt_node_t* nodes[16];
} gnt_nibble_t;

typedef struct gnt_node // Represents the last 4 bits of a byte
{
    bool occupied;
    uint8_t children;
    gnt_data_t data;
    gnt_nibble_t* nibbles[16];
} gnt_node_t;

typedef struct gnt_trie
{
    mtx_t mutex;
    uint8_t children;
    gnt_nibble_t* nibbles[16];
    gnt_accessor_t accessor;
    gnt_deallocator_t deallocator;
} gnt_trie_t;

enum
{
    END,
    CONTINUE,
    STOP
};

static gnt_status_t _gnt_accessor_default(gnt_byte_t* byte, gnt_key_t key, gnt_index_t index);
static void _gnt_destroy_recursive_nibbles(gnt_nibble_t** nibbles, uint8_t children, gnt_deallocator_t deallocator);
static void _gnt_destroy_recursive_nodes(gnt_node_t** nodes, uint8_t children, gnt_deallocator_t deallocator);
static gnt_status_t _gnt_delete_recursive(gnt_byte_t* byte, gnt_nibble_t** nibbles, gnt_key_t key, gnt_index_t index, gnt_accessor_t accessor, gnt_deallocator_t deallocator);

gnt_trie_t* gnt_create(gnt_cfg_t* cfg)
{
    gnt_accessor_t accessor;
    gnt_deallocator_t deallocator;
    
    if (cfg)
    {
        accessor = cfg->accessor ? cfg->accessor : _gnt_accessor_default;
        deallocator = cfg->deallocator;
    }
    else
    {
        accessor = _gnt_accessor_default;
        deallocator = NULL;
    }
    
    gnt_trie_t* trie = calloc(1, sizeof(gnt_trie_t));
    
    if (trie)
    {
        if (thrd_success != mtx_init(&trie->mutex, mtx_plain))
        {
            free(trie);
            return NULL;
        }

        trie->accessor = accessor;
        trie->deallocator = deallocator;
    }
    
    return trie;
}

gnt_status_t gnt_destroy(gnt_trie_t* trie)
{
    if (!trie) return -1;
    
    _gnt_destroy_recursive_nibbles(trie->nibbles, trie->children, trie->deallocator);
    
    mtx_destroy(&trie->mutex);
    free(trie);
    
    return 0;
}

gnt_status_t gnt_insert(gnt_trie_t* trie, gnt_key_t key, gnt_data_t data)
{
    if (!trie) return -1;
    mtx_lock(&trie->mutex);
    
    gnt_nibble_t** nibbles = trie->nibbles;
    gnt_node_t** nodes;
    gnt_node_t* node;
    gnt_byte_t byte;
    gnt_index_t index = 0;
    uint8_t* children = &(trie->children);
    
    while (0 == trie->accessor(&byte, key, index++))
    {
        gnt_byte_t high_nibble = GNT_HIGH_NIBBLE(byte);
        gnt_byte_t low_nibble = GNT_LOW_NIBBLE(byte);
        
        if (!nibbles[high_nibble])
        {
            nibbles[high_nibble] = calloc(1, sizeof(gnt_nibble_t));
            (*children)++;
        }

        nodes = nibbles[high_nibble]->nodes;
        children = &(nibbles[high_nibble]->children);
        
        if (!nodes[low_nibble])
        {
            nodes[low_nibble] = calloc(1, sizeof(gnt_node_t));
            (*children)++;
        }

        nibbles = nodes[low_nibble]->nibbles;
        node = nodes[low_nibble];
        children = &(node->children);
    }
    
    if (node->occupied && trie->deallocator)
    {
        trie->deallocator(node->data);
    }
    
    node->data = data;
    node->occupied = true;

    mtx_unlock(&trie->mutex);
    return 0;
}

gnt_data_t gnt_search(gnt_trie_t* trie, gnt_key_t key)
{
    if (!trie) return -1;
    mtx_lock(&trie->mutex);
    
    gnt_nibble_t** nibbles = trie->nibbles;
    gnt_node_t** nodes;
    gnt_node_t* node;
    gnt_byte_t byte;
    gnt_index_t index = 0;
    
    while (0 == trie->accessor(&byte, key, index++))
    {
        gnt_byte_t high_nibble = GNT_HIGH_NIBBLE(byte);
        gnt_byte_t low_nibble = GNT_LOW_NIBBLE(byte);
        
        if (!nibbles[high_nibble])
        {
            mtx_unlock(&trie->mutex);
            return 0;
        }

        nodes = nibbles[high_nibble]->nodes;
        
        if (!nodes[low_nibble])
        {
            mtx_unlock(&trie->mutex);
            return 0;
        }

        nibbles = nodes[low_nibble]->nibbles;
        node = nodes[low_nibble];
    }

    gnt_data_t data = node->data;

    mtx_unlock(&trie->mutex);
    return data;
}

gnt_status_t gnt_delete(gnt_trie_t* trie, gnt_key_t key)
{
    if (!trie) return -1;
    mtx_lock(&trie->mutex);
    
    uint8_t child_byte;
    
    if (CONTINUE == _gnt_delete_recursive(&child_byte, trie->nibbles, key, 0, trie->accessor, trie->deallocator))
    {
        if (trie->children)
        {
            free(trie->nibbles[GNT_HIGH_NIBBLE(child_byte)]);
            trie->nibbles[GNT_HIGH_NIBBLE(child_byte)] = NULL;
            trie->children--;
        }
    }
    
    mtx_unlock(&trie->mutex);
    return 0;
}

gnt_status_t gnt_accessor_string(gnt_byte_t* byte, gnt_key_t key, gnt_index_t index)
{
    char* str = (char*) key;
    
    if (index >= strlen(str))
    {
        return -1;
    }
    
    *byte = str[index];
    return 0;
}

static gnt_status_t _gnt_accessor_default(gnt_byte_t* byte, gnt_key_t key, gnt_index_t index)
{
    uint8_t digits = 0;
    gnt_key_t temp = key;

    do
    {
        digits++;
        temp >>= 8; // division by 256
    } while (temp > 0);

    if (index >= digits)
    {
        return -1;
    }

    for (uint8_t i = 0; i < digits - index - 1; i++)
    {
        key >>= 8; // division by 256
    }

    *byte = key & 255; // modulo 256
    return 0;
}

static void _gnt_destroy_recursive_nibbles(gnt_nibble_t** nibbles, uint8_t children, gnt_deallocator_t deallocator)
{
    for (uint8_t i = 0; children && i < 16; i++)
    {
        if (nibbles[i])
        {
            _gnt_destroy_recursive_nodes(nibbles[i]->nodes, nibbles[i]->children, deallocator);
            free(nibbles[i]);
            nibbles[i] = 0;
            children--;
        }
    }
}

static void _gnt_destroy_recursive_nodes(gnt_node_t** nodes, uint8_t children, gnt_deallocator_t deallocator)
{
    for (uint8_t i = 0; children && i < 16; i++)
    {
        if (nodes[i])
        {
            _gnt_destroy_recursive_nibbles(nodes[i]->nibbles, nodes[i]->children, deallocator);
            
            if (deallocator)
            {
                deallocator(nodes[i]->data);
            }
            
            free(nodes[i]);
            nodes[i] = 0;
            children--;
        }
    }
}

static gnt_status_t _gnt_delete_recursive(gnt_byte_t* byte, gnt_nibble_t** nibbles, gnt_key_t key, gnt_index_t index, gnt_accessor_t accessor, gnt_deallocator_t deallocator)
{
    if (0 != accessor(byte, key, index))
    {
        return END;
    }
    
    gnt_byte_t high_nibble = GNT_HIGH_NIBBLE(*byte);
    gnt_byte_t low_nibble = GNT_LOW_NIBBLE(*byte);
    
    gnt_nibble_t* nibble = nibbles[high_nibble];
    gnt_node_t* node = nibble->nodes[low_nibble];
    
    uint8_t child_byte = 0;
    gnt_status_t status = _gnt_delete_recursive(&child_byte, node->nibbles, key, index + 1, accessor, deallocator);

    if (STOP == status)
    {
        return STOP;
    }
    
    if (END == status)
    {
        if (deallocator)
        {
            deallocator(node->data);
        }
        
        node->data = 0;
        node->occupied = false;
        
        if (node->children)
        {
            return STOP;
        }
    }
    
    free(node->nibbles[GNT_HIGH_NIBBLE(child_byte)]);
    node->nibbles[GNT_HIGH_NIBBLE(child_byte)] = NULL;
    
    if (node->children)
    {
        node->children--;
    }
    
    if (node->children)
    {
        return STOP;
    }
    
    free(nibble->nodes[low_nibble]);
    nibble->nodes[low_nibble] = NULL;
    nibble->children--;
    
    if (nibble->children)
    {
        return STOP;
    }
    
    return CONTINUE;
}
