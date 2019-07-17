# Harbol

## Introduction

**Harbol** is a collection of data structure and miscellaneous libraries, similar in nature to C++'s Boost, STL, and GNOME's GLib; it is meant to be a smaller and more lightweight collection of data structures, code systems, and convenience software.


### Features

* Variant Type - supports any type of values and their type IDs
* C++ style String type
* Vector / Dynamic-Array
* String-based Hash Table
* Singly Linked List
* Doubly Linked List
* Byte Buffer
* Tuple type - convertible to structs, can also be packed.
* Memory Pool - returns any size and can defrag itself.
* Object Pool - like the memory pool but for fixed size data.
* Weighted Graph
* N-ary Tree
* Ordered String-based Hash Table
* JSON-like Key-Value Configuration File Parser - allows retrieving data from keys through python-style pathing.
* Plugin Manager - designed to be wrapped around to provide an easy-to-setup plugin API and plugin SDK.
* Fixed Size floating-point types.
* 


## Usage

```c
#include "harbol.h"

int main(const int argc, char *argv[])
{
	struct HarbolString str = harbol_string_create("my initial string!");
	harbol_string_add_cstr(&str, "and another string concatenated!");
	harbol_string_clear(&str);
	
	struct HarbolVector vec = harbol_vector_create(sizeof(float32_t));
	harbol_vector_insert(&vec, &(float32_t){2.f});
	harbol_vector_insert(&vec, &(float32_t){3.f});
	harbol_vector_insert(&vec, &(float32_t){4.f});
	const float32_t f = *(float32_t *)harbol_vector_get(&vec, 1);
	harbol_vector_clear(&vec, NULL);
	
	struct HarbolMap *ptrmap = harbol_map_new(sizeof(float64_t));
	harbol_map_insert(ptrmap, "style", &(float64_t){2.3553});
	harbol_map_insert(ptrmap, "border", &(float64_t){12.995});
	harbol_map_free(&ptrmap, NULL);
}
```

## Contributing

To submit a patch, first file an issue and/or present a pull request.

## Help

If you need help or have any question, make an issue on the github repository.
Simply drop a message or your question and you'll be reached in no time!

## Installation

### Requirements

C99 compliant compiler and libc implementation with stdlib.h, stdio.h, and stddef.h.

### Installation

To build the library, simply run `make harbol_static` which will make the static library version of libharbol.
for a shared library version, run `make harbol_shared`. to clean up the `.o` files, run `make clean`.

### Testing

For testing code changes or additions, simply run `make test` with `test_suite.c` in the repository which will build an executable called `harbol_testprogram`.


## Credits

* Khanno Hanna - main developer of libharbol.


## Contact

I can be contacted at edyonan@yahoo.com. No soliciting or spam.


## License

This project is licensed under Apache License.
