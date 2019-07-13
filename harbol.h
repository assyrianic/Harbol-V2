#ifndef HARBOL_INCLUDED
#	define HARBOL_INCLUDED
#	define HARBOL_VERSION_MAJOR    2
#	define HARBOL_VERSION_MINOR    0
#	define HARBOL_VERSION_PATCH    4
#	define HARBOL_VERSION_PHASE    'B'
#	define STR_HELPER(x)    #x
#	define STR(x)    STR_HELPER(x)
#	define HARBOL_VERSION_STRING \
			STR(HARBOL_VERSION_MAJOR) "." STR(HARBOL_VERSION_MINOR) "." STR(HARBOL_VERSION_PATCH) " " STR(HARBOL_VERSION_PHASE)

#include "harbol_common_defines.h"


#ifndef C99
#	error "Harbol requires a C99 compliant compiler with at least stdbool.h, inttypes.h, compound literals, and designated initializers."
#endif

#include "harbol_common_includes.h"

#ifdef __cplusplus
extern "C" {
#endif


/************* General-Purpose Memory Pool *************/
#include "allocators/mempool/mempool.h"
/*******************************************************/

/************* Fast & Efficient Object Pool *************/
#include "allocators/objpool/objpool.h"
/******************************************************************/

/************* Simple & Efficient Linear Allocator Pool *************/
#include "allocators/cache/cache.h"
/********************************************************************/

/************* C++ Style Automated String *************/
#include "stringobj/stringobj.h"
/******************************************************/

/************* Dynamic Array *************/
#include "vector/vector.h"
/*****************************************/

/************* Singly Linked List *************/
#include "unilist/unilist.h"
/**********************************************/

/************* Doubly Linked List *************/
#include "bilist/bilist.h"
/**********************************************/

/************* Tuple Type *************/
#include "tuple/tuple.h"
/**************************************/

/************* Byte Buffer *************/
#include "bytebuffer/bytebuffer.h"
/**************************************/

/************* String-based Hash Table *************/
#include "map/map.h"
/***************************************************/

/************* Weight Adjacency List Graph *************/
#include "graph/graph.h"
/*******************************************************/

/************* n-Ary Tree *************/
#include "tree/tree.h"
/**************************************/

/************* Linked Hashmap (preserves insertion order) *************/
#include "linkmap/linkmap.h"
/**********************************************************************/

/************* Variant Type *************/
#include "variant/variant.h"
/****************************************/

/************* JSON-like Config Parser *************/
#include "cfg/cfg.h"
/***************************************************/

/************* Shared Library Plugin Manager *************/
#include "plugins/plugins.h"
/*********************************************************/

#ifdef __cplusplus
}
#endif

#endif /* HARBOL_INCLUDED */
