#ifndef HARBOL_MEMPOOL_INCLUDED
#	define HARBOL_MEMPOOL_INCLUDED

#include "../../harbol_common_defines.h"
#include "../../harbol_common_includes.h"

#ifdef __cplusplus
extern "C" {
#endif


struct HarbolMemNode {
	size_t Size;
	struct HarbolMemNode *Next, *Prev; //*Left, *Rite;
};

struct HarbolMemPool {
	struct {
		struct HarbolMemNode *Head, *Tail;
		size_t Len, MaxNodes;
		bool AutoDefrag : 1;
	} FreeList;
	
	struct {
		uint8_t *Mem, *Base;
		size_t Size;
	} Stack;
};


HARBOL_EXPORT struct HarbolMemPool harbol_mempool_create(size_t bytes);
HARBOL_EXPORT NO_NULL struct HarbolMemPool harbol_mempool_from_buffer(void *buf, size_t bytes);
HARBOL_EXPORT NO_NULL bool harbol_mempool_clear(struct HarbolMemPool *mempool);

HARBOL_EXPORT NO_NULL void *harbol_mempool_alloc(struct HarbolMemPool *mempool, size_t bytes);
HARBOL_EXPORT NEVER_NULL(1) void *harbol_mempool_realloc(struct HarbolMemPool *mempool, void *ptr, size_t bytes);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_mempool_free(struct HarbolMemPool *mempool, void *ptr);
HARBOL_EXPORT NO_NULL bool harbol_mempool_cleanup(struct HarbolMemPool *mempool, void **ptrref);

HARBOL_EXPORT NO_NULL size_t harbol_mempool_mem_remaining(const struct HarbolMemPool *mempool);
HARBOL_EXPORT NO_NULL bool harbol_mempool_defrag(struct HarbolMemPool *mempool);
HARBOL_EXPORT NO_NULL void harbol_mempool_toggle_auto_defrag(struct HarbolMemPool *mempool);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_MEMPOOL_INCLUDED */
