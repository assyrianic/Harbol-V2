#ifndef HARBOL_MAP_INCLUDED
#	define HARBOL_MAP_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"
#include "../stringobj/stringobj.h"
#include "../vector/vector.h"

#ifndef MAP_DEFAULT_SIZE
#	define MAP_DEFAULT_SIZE    8
#endif


struct HarbolKeyVal {
	struct HarbolString key;
	uint8_t *data;
};

HARBOL_EXPORT NO_NULL struct HarbolKeyVal *harbol_kvpair_new(const char cstr[], void *data, size_t datasize);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_kvpair_free(struct HarbolKeyVal **kvpairref, void dtor(void**));


struct HarbolMap {
	// buckets is a vector of vectors of keyval pointers.
	struct HarbolVector *buckets;
	size_t len, count, datasize;
};

#define EMPTY_HARBOL_MAP    {NULL,0,0,0}

HARBOL_EXPORT struct HarbolMap *harbol_map_new(size_t datasize);
HARBOL_EXPORT struct HarbolMap harbol_map_create(size_t datasize);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_map_clear(struct HarbolMap *map, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_map_free(struct HarbolMap **mapref, void dtor(void**));

HARBOL_EXPORT NO_NULL bool harbol_map_insert(struct HarbolMap *map, const char key[], void *val);
HARBOL_EXPORT NO_NULL bool harbol_map_insert_kv(struct HarbolMap *map, struct HarbolKeyVal *kv);

HARBOL_EXPORT NO_NULL void *harbol_map_get(const struct HarbolMap *map, const char key[]);
HARBOL_EXPORT NO_NULL struct HarbolKeyVal *harbol_map_get_kv(const struct HarbolMap *map, const char key[]);
HARBOL_EXPORT NO_NULL bool harbol_map_set(struct HarbolMap *map, const char key[], void *val);

HARBOL_EXPORT NO_NULL bool harbol_map_has_key(const struct HarbolMap *map, const char key[]);
HARBOL_EXPORT NO_NULL bool harbol_map_rehash(struct HarbolMap *map, size_t new_len);
HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_map_del(struct HarbolMap *map, const char key[], void dtor(void**));
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_MAP_INCLUDED */
