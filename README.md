# Generic Nibble Trie (GNT) Library

## Overview

The **Generic Nibble Trie (GNT)** library is designed for efficient storage and retrieval of data using nibble tries, also known as prefix trees. This library is type-agnostic, allowing users to insert and search for data of various types using flexible key representations.

## Features
- **Generic Key-Value Storage:** Supports different data types (integers, floats, and pointers) for both keys and values using a unified `gnt_key_t` and `gnt_data_t` type.
- **Memory Management:** Offers user-defined deallocation functions for managing custom data structures.

## Getting Started

### Prerequisites
To use the GNT library, ensure you have a C compiler installed and the necessary tools to compile and link C programs.

### Installation

1. Clone the GNT library source files into your project directory.
2. Include the **gnt.h** header file in your project:
```c
#include "gnt.h"
```

### Compilation
To compile your program with the GNT library, ensure you link both the **gnt.c** and **gnt.h** files with your program:

```bash
gcc -o your_program your_program.c gnt.c
```

### Basic Usage Example

```c
#include <stdio.h>
#include "gnt.h"

int main()
{
  // Create a nibble trie
  gnt_trie_t* trie = gnt_create(NULL);

  // Insert key-value pairs
  gnt_insert(trie, GNT_KEY(1), GNT_DATA(100));
  gnt_insert(trie, GNT_KEY(2), GNT_DATA(200));

  // Search for a value by key
  printf("Value for key 1: %ld\n", (long) gnt_search(trie, GNT_KEY(1)));

  // Delete a key-value pair
  gnt_delete(trie, GNT_KEY(1));

  // Destroy the trie when done
  gnt_destroy(trie);

  return 0;
}
```

### API Documentation

#### Table Management
- `gnt_trie_t* gnt_create(gnt_cfg_t* cfg);`  
  Creates and returns a new nibble trie.

- `gnt_status_t gnt_destroy(gnt_trie_t* trie);`  
  Destroys the trie and frees all allocated memory using a custom deallocator if provided.

#### Data Operations
- `gnt_status_t gnt_insert(gnt_trie_t* trie, gnt_key_t key, gnt_data_t data);`  
  Inserts a key-value pair into the trie.

- `gnt_data_t gnt_search(gnt_trie_t* trie, gnt_key_t key);`  
  Searches and returns the value associated with the given key, or 0 if not found.

- `gnt_status_t gnt_delete(gnt_trie_t* trie, gnt_key_t key);`  
  Deletes the key-value pair from the trie.

#### Conversion Macros
- `GNT_DATA(data)`  
  Converts various data types (integers, floats, pointers) to `gnt_data_t`, which is used in the nibble trie.

- `GNT_KEY(key)`  
  Converts various data types to `gnt_key_t`, which is used as a key in the nibble trie.

## License

The GNT library is released under the **MIT License**. You are free to use, modify, and distribute it under the terms of the license. See the [MIT License](https://opensource.org/licenses/MIT) for more details.

## Author

This library was developed by **Laurent Mailloux-Bourassa**.