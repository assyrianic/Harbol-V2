#ifndef HARBOL_OBJPOOL_INCLUDED
#	define HARBOL_OBJPOOL_INCLUDED

#include "../../harbol_common_defines.h"
#include "../../harbol_common_includes.h"

#ifdef __cplusplus
extern "C" {
#endif


struct HarbolObjPool {
	uint8_t
		*Mem, // Beginning of memory pool
		*Next // Num of next free block
	;
	size_t
		Size, // Num of blocks.
		ObjSize, // Size of each block
		FreeBlocks // Num of remaining blocks
	;
};

HARBOL_EXPORT struct HarbolObjPool harbol_objpool_create(size_t objsize, size_t len);
HARBOL_EXPORT NO_NULL struct HarbolObjPool harbol_objpool_from_buffer(void *buf, size_t objsize, size_t len);
HARBOL_EXPORT NO_NULL bool harbol_objpool_clear(struct HarbolObjPool *objpool);

HARBOL_EXPORT NO_NULL void *harbol_objpool_alloc(struct HarbolObjPool *objpool);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_objpool_free(struct HarbolObjPool *objpool, void *ptr);
HARBOL_EXPORT NO_NULL bool harbol_objpool_cleanup(struct HarbolObjPool *objpool, void **ptrref);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_OBJPOOL_INCLUDED */
