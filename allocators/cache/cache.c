#include "cache.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolCache harbol_cache_create(const size_t size)
{
	struct HarbolCache cache = EMPTY_HARBOL_CACHE;
	if( size==0 )
		return cache;
	else {
		cache.base = calloc(size, sizeof *cache.base);
		if( cache.base==NULL ) {
			return cache;
		} else {
			cache.offset = cache.base + size;
			return cache;
		}
	}
}

HARBOL_EXPORT struct HarbolCache harbol_cache_from_buffer(void *const buf, const size_t size)
{
	struct HarbolCache cache = EMPTY_HARBOL_CACHE;
	if( size==0 )
		return cache;
	else {
		cache.base = buf;
		cache.offset = cache.base + size;
		return cache;
	}
}

HARBOL_EXPORT bool harbol_cache_clear(struct HarbolCache *const cache)
{
	if( cache->base==NULL )
		return false;
	else {
		free(cache->base);
		*cache = (struct HarbolCache)EMPTY_HARBOL_CACHE;
		return true;
	}
}

HARBOL_EXPORT void *harbol_cache_alloc(struct HarbolCache *const cache, const size_t size)
{
	if( cache->base==NULL || size==0 || size > harbol_cache_remaining(cache) )
		return NULL;
	else {
		cache->offset -= harbol_align_size(size, sizeof(uintptr_t));
		return cache->offset;
	}
}

HARBOL_EXPORT size_t harbol_cache_remaining(const struct HarbolCache *cache)
{
	return (uintptr_t)cache->offset - (uintptr_t)cache->base;
}
