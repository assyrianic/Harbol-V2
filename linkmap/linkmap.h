#ifndef HARBOL_LINKMAP_INCLUDED
#	define HARBOL_LINKMAP_INCLUDED

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"
#include "../map/map.h"

#ifdef __cplusplus
extern "C" {
#endif


struct HarbolLinkMap {
	struct HarbolMap Map;
	struct HarbolVector Vec;
};

#define EMPTY_HARBOL_LINKMAP    { EMPTY_HARBOL_VECTOR, EMPTY_HARBOL_MAP }

HARBOL_EXPORT struct HarbolLinkMap *harbol_linkmap_new(size_t datasize);
HARBOL_EXPORT struct HarbolLinkMap harbol_linkmap_create(size_t datasize);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_linkmap_clear(struct HarbolLinkMap *map, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_linkmap_free(struct HarbolLinkMap **mapref, void dtor(void**));

HARBOL_EXPORT NO_NULL size_t harbol_linkmap_count(const struct HarbolLinkMap *map);
HARBOL_EXPORT NO_NULL bool harbol_linkmap_has_key(const struct HarbolLinkMap *map, const char key[]);

HARBOL_EXPORT NO_NULL bool harbol_linkmap_insert(struct HarbolLinkMap *map, const char key[], void *val);
HARBOL_EXPORT NO_NULL bool harbol_linkmap_insert_kv(struct HarbolLinkMap *map, struct HarbolKeyVal *kv);

HARBOL_EXPORT NO_NULL void *harbol_linkmap_key_get(const struct HarbolLinkMap *map, const char key[]);
HARBOL_EXPORT NO_NULL void *harbol_linkmap_index_get(const struct HarbolLinkMap *map, uindex_t index);

HARBOL_EXPORT NO_NULL struct HarbolKeyVal *harbol_linkmap_key_get_kv(const struct HarbolLinkMap *map, const char key[]);
HARBOL_EXPORT NO_NULL struct HarbolKeyVal *harbol_linkmap_index_get_kv(const struct HarbolLinkMap *map, uindex_t index);

HARBOL_EXPORT NO_NULL bool harbol_linkmap_key_set(struct HarbolLinkMap *map, const char key[], void *val);
HARBOL_EXPORT NO_NULL bool harbol_linkmap_index_set(struct HarbolLinkMap *map, uindex_t index, void *val);

HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_linkmap_key_del(struct HarbolLinkMap *map, const char key[], void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_linkmap_index_del(struct HarbolLinkMap *map, uindex_t index, void dtor(void**));

HARBOL_EXPORT NO_NULL index_t harbol_linkmap_get_key_index(const struct HarbolLinkMap *linkmap, const char key[]);
HARBOL_EXPORT NO_NULL index_t harbol_linkmap_get_node_index(const struct HarbolLinkMap *linkmap, struct HarbolKeyVal *kv);
HARBOL_EXPORT NO_NULL index_t harbol_linkmap_get_val_index(const struct HarbolLinkMap *linkmap, void *val);

HARBOL_EXPORT NO_NULL void *harbol_linkmap_get_iter(const struct HarbolLinkMap *map);
HARBOL_EXPORT NO_NULL void *harbol_linkmap_get_iter_end_count(const struct HarbolLinkMap *map);
HARBOL_EXPORT NO_NULL void *harbol_linkmap_get_iter_end_len(const struct HarbolLinkMap *map);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_LINKMAP_INCLUDED */
