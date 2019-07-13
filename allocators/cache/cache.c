#include "cache.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolCache harbol_cache_create(const size_t size)
{
	struct HarbolCache cache = {NULL,NULL};
	if( size==0 )
		return cache;
	else {
		cache.Base = calloc(size, sizeof *cache.Base);
		if( cache.Base==NULL ) {
			return cache;
		} else {
			cache.Offs = cache.Base + size;
			return cache;
		}
	}
}

HARBOL_EXPORT struct HarbolCache harbol_cache_from_buffer(void *const buf, const size_t size)
{
	struct HarbolCache cache = {NULL,NULL};
	if( size==0 )
		return cache;
	else {
		cache.Base = buf;
		cache.Offs = cache.Base + size;
		return cache;
	}
}

HARBOL_EXPORT bool harbol_cache_clear(struct HarbolCache *const cache)
{
	if( cache->Base==NULL )
		return false;
	else {
		free(cache->Base), cache->Base=NULL;
		cache->Offs = NULL;
		return true;
	}
}

HARBOL_EXPORT void *harbol_cache_alloc(struct HarbolCache *const cache, const size_t size)
{
	if( cache->Base==NULL || size==0 || size > (uintptr_t)cache->Offs - (uintptr_t)cache->Base )
		return NULL;
	else {
		cache->Offs -= size; //harbol_align_size(size, sizeof(uintptr_t));
		return cache->Offs;
	}
}

HARBOL_EXPORT size_t harbol_cache_remaining(const struct HarbolCache *cache)
{
	return (uintptr_t)cache->Offs - (uintptr_t)cache->Base;
}
