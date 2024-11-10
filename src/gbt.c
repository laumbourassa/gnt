/*
 * gbt.c - Generic Bitwise Trie implementation
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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "gbt.h"

#define GBT_HIGH_NIBBLE(byte)           (byte >> 4)
#define GBT_LOW_NIBBLE(byte)            (byte & 0x0F)
#define GBT_SELECT_KEY_BYTE(key, index) (key >> (8 * index))

typedef struct gbt_node gbt_node_t;

typedef struct gbt_nibble gbt_nibble_t;
typedef struct gbt_nibble // Represents the first 4 bits of a byte
{
    uint8_t children;
    gbt_node_t* nodes[16];
} gbt_nibble_t;

typedef struct gbt_node // Represents the last 4 bits of a byte
{
    bool occupied;
    uint8_t children;
    gbt_data_t data;
    gbt_nibble_t* nibbles[16];
} gbt_node_t;

typedef struct gbt_trie
{
    uint8_t children;
    gbt_nibble_t* nibbles[16];
    gbt_accessor_t accessor;
    gbt_deallocator_t deallocator;
} gbt_trie_t;

enum
{
    END,
    CONTINUE,
    STOP
};

static gbt_status_t _gbt_accessor_default(gbt_byte_t* byte, gbt_key_t key, gbt_index_t index);
static void _gbt_destroy_recursive_nibbles(gbt_nibble_t** nibbles, uint8_t children, gbt_deallocator_t deallocator);
static void _gbt_destroy_recursive_nodes(gbt_node_t** nodes, uint8_t children, gbt_deallocator_t deallocator);
static gbt_status_t _gbt_delete_recursive(gbt_byte_t* byte, gbt_nibble_t** nibbles, gbt_key_t key, gbt_index_t index, gbt_accessor_t accessor, gbt_deallocator_t deallocator);

gbt_trie_t* gbt_create(gbt_cfg_t* cfg)
{
    gbt_accessor_t accessor;
    gbt_deallocator_t deallocator;
    
    if (cfg)
    {
        accessor = cfg->accessor ? cfg->accessor : _gbt_accessor_default;
        deallocator = cfg->deallocator;
    }
    else
    {
        accessor = _gbt_accessor_default;
        deallocator = NULL;
    }
    
    gbt_trie_t* trie = calloc(1, sizeof(gbt_trie_t));
    
    if (trie)
    {
        trie->accessor = accessor;
        trie->deallocator = deallocator;
    }
    
    return trie;
}

gbt_status_t gbt_destroy(gbt_trie_t* trie)
{
    if (!trie) return -1;
    
    _gbt_destroy_recursive_nibbles(trie->nibbles, trie->children, trie->deallocator);
    free(trie);
    
    return 0;
}

gbt_status_t gbt_insert(gbt_trie_t* trie, gbt_key_t key, gbt_data_t data)
{
    if (!trie) return -1;
    
    gbt_nibble_t** nibbles = trie->nibbles;
    gbt_node_t** nodes;
    gbt_node_t* node;
    gbt_byte_t byte;
    gbt_index_t index = 0;
    uint8_t* children = &(trie->children);
    
    while (0 == trie->accessor(&byte, key, index++))
    {
        gbt_byte_t high_nibble = GBT_HIGH_NIBBLE(byte);
        gbt_byte_t low_nibble = GBT_LOW_NIBBLE(byte);
        
        if (!nibbles[high_nibble])
        {
            nibbles[high_nibble] = calloc(1, sizeof(gbt_nibble_t));
            (*children)++;
        }

        nodes = nibbles[high_nibble]->nodes;
        children = &(nibbles[high_nibble]->children);
        
        if (!nodes[low_nibble])
        {
            nodes[low_nibble] = calloc(1, sizeof(gbt_node_t));
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
    
    return 0;
}

gbt_data_t gbt_search(gbt_trie_t* trie, gbt_key_t key)
{
    if (!trie) return -1;
    
    gbt_nibble_t** nibbles = trie->nibbles;
    gbt_node_t** nodes;
    gbt_node_t* node;
    gbt_byte_t byte;
    gbt_index_t index = 0;
    
    while (0 == trie->accessor(&byte, key, index++))
    {
        gbt_byte_t high_nibble = GBT_HIGH_NIBBLE(byte);
        gbt_byte_t low_nibble = GBT_LOW_NIBBLE(byte);
        
        if (!nibbles[high_nibble])
        {
            return 0;
        }

        nodes = nibbles[high_nibble]->nodes;
        
        if (!nodes[low_nibble])
        {
            return 0;
        }

        nibbles = nodes[low_nibble]->nibbles;
        node = nodes[low_nibble];
    }
    
    return node->data;
}

gbt_status_t gbt_delete(gbt_trie_t* trie, gbt_key_t key)
{
    if (!trie) return -1;
    
    uint8_t child_byte;
    
    if (CONTINUE == _gbt_delete_recursive(&child_byte, trie->nibbles, key, 0, trie->accessor, trie->deallocator))
    {
        if (trie->children)
        {
            free(trie->nibbles[GBT_HIGH_NIBBLE(child_byte)]);
            trie->nibbles[GBT_HIGH_NIBBLE(child_byte)] = NULL;
            trie->children--;
        }
    }
    
    return 0;
}

gbt_status_t gbt_accessor_string(gbt_byte_t* byte, gbt_key_t key, gbt_index_t index)
{
    char* str = (char*) key;
    
    if (index >= strlen(str))
    {
        return -1;
    }
    
    *byte = str[index];
    return 0;
}

static gbt_status_t _gbt_accessor_default(gbt_byte_t* byte, gbt_key_t key, gbt_index_t index)
{
    uint8_t digits = 0;
    gbt_key_t temp = key;

    do
    {
        digits++;
        temp /= 10;
    } while (temp > 0);

    if (index >= digits)
    {
        return -1;
    }

    for (uint8_t i = 0; i < digits - index - 1; i++)
    {
        key /= 10;
    }

    *byte = key % 10;
    return 0;
}

static void _gbt_destroy_recursive_nibbles(gbt_nibble_t** nibbles, uint8_t children, gbt_deallocator_t deallocator)
{
    for (uint8_t i = 0; children && i < 16; i++)
    {
        if (nibbles[i])
        {
            _gbt_destroy_recursive_nodes(nibbles[i]->nodes, nibbles[i]->children, deallocator);
            free(nibbles[i]);
            nibbles[i] = 0;
            children--;
        }
    }
}

static void _gbt_destroy_recursive_nodes(gbt_node_t** nodes, uint8_t children, gbt_deallocator_t deallocator)
{
    for (uint8_t i = 0; children && i < 16; i++)
    {
        if (nodes[i])
        {
            _gbt_destroy_recursive_nibbles(nodes[i]->nibbles, nodes[i]->children, deallocator);
            
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

static gbt_status_t _gbt_delete_recursive(gbt_byte_t* byte, gbt_nibble_t** nibbles, gbt_key_t key, gbt_index_t index, gbt_accessor_t accessor, gbt_deallocator_t deallocator)
{
    if (0 != accessor(byte, key, index))
    {
        return END;
    }
    
    gbt_byte_t high_nibble = GBT_HIGH_NIBBLE(*byte);
    gbt_byte_t low_nibble = GBT_LOW_NIBBLE(*byte);
    
    gbt_nibble_t* nibble = nibbles[high_nibble];
    gbt_node_t* node = nibble->nodes[low_nibble];
    
    uint8_t child_byte = 0;
    gbt_status_t status = _gbt_delete_recursive(&child_byte, node->nibbles, key, index + 1, accessor, deallocator);

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
    
    free(node->nibbles[GBT_HIGH_NIBBLE(child_byte)]);
    node->nibbles[GBT_HIGH_NIBBLE(child_byte)] = NULL;
    
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