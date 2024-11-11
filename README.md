# Generic Bitwise Trie (GBT) Library

## Overview

The **Generic Bitwise Trie (GBT)** library is designed for efficient storage and retrieval of data using bitwise tries, also known as prefix trees. This library is type-agnostic, allowing users to insert and search for data of various types using flexible key representations.

## Features
- **Generic Key-Value Storage:** Supports different data types (integers, floats, and pointers) for both keys and values using a unified `gbt_key_t` and `gbt_data_t` type.
- **Memory Management:** Offers user-defined deallocation functions for managing custom data structures.

## Getting Started

### Prerequisites
To use the GBT library, ensure you have a C compiler installed and the necessary tools to compile and link C programs.

### Installation

1. Clone the GBT library source files into your project directory.
2. Include the **gbt.h** header file in your project:
```c
#include "gbt.h"
```

### Compilation
To compile your program with the GBT library, ensure you link both the **gbt.c** and **gbt.h** files with your program:

```bash
gcc -o your_program your_program.c gbt.c
```

### Basic Usage Example

```c
#include <stdio.h>
#include "gbt.h"

int main()
{
  // Create a bitwise trie
  gbt_trie_t* trie = gbt_create(NULL);

  // Insert key-value pairs
  gbt_insert(trie, GBT_KEY(1), GBT_DATA(100));
  gbt_insert(trie, GBT_KEY(2), GBT_DATA(200));

  // Search for a value by key
  printf("Value for key 1: %ld\n", (long) gbt_search(trie, GBT_KEY(1)));

  // Delete a key-value pair
  gbt_delete(trie, GBT_KEY(1));

  // Destroy the trie when done
  gbt_destroy(trie);

  return 0;
}
```

### API Documentation

#### Table Management
- `gbt_trie_t* gbt_create(gbt_cfg_t* cfg);`  
  Creates and returns a new bitwise trie.

- `gbt_status_t gbt_destroy(gbt_trie_t* trie);`  
  Destroys the trie and frees all allocated memory using a custom deallocator if provided.

#### Data Operations
- `gbt_status_t gbt_insert(gbt_trie_t* trie, gbt_key_t key, gbt_data_t data);`  
  Inserts a key-value pair into the trie.

- `gbt_data_t gbt_search(gbt_trie_t* trie, gbt_key_t key);`  
  Searches and returns the value associated with the given key, or 0 if not found.

- `gbt_status_t gbt_delete(gbt_trie_t* trie, gbt_key_t key);`  
  Deletes the key-value pair from the trie.

#### Conversion Macros
- `GBT_DATA(data)`  
  Converts various data types (integers, floats, pointers) to `gbt_data_t`, which is used in the bitwise trie.

- `GBT_KEY(key)`  
  Converts various data types to `gbt_key_t`, which is used as a key in the bitwise trie.

## License

The GBT library is released under the **MIT License**. You are free to use, modify, and distribute it under the terms of the license. See the [MIT License](https://opensource.org/licenses/MIT) for more details.

## Author

This library was developed by **Laurent Mailloux-Bourassa**.