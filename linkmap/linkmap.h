#ifndef HARBOL_LINKMAP_INCLUDED
#	define HARBOL_LINKMAP_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"
#include "../map/map.h"


struct HarbolLinkMap {
	struct HarbolMap map;
	struct HarbolVector vec;
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

#ifdef C11
#	define harbol_linkmap_get(map, key)     _Generic((key)+0, \
												int8_t : harbol_linkmap_index_get, \
												uint8_t : harbol_linkmap_index_get, \
												int16_t : harbol_linkmap_index_get, \
												uint16_t : harbol_linkmap_index_get, \
												int32_t : harbol_linkmap_index_get, \
												uint32_t : harbol_linkmap_index_get, \
												int64_t : harbol_linkmap_index_get, \
												uint64_t : harbol_linkmap_index_get, \
												char* : harbol_linkmap_key_get, \
												const char* : harbol_linkmap_key_get) \
											((map), (key))
											
#	define harbol_linkmap_get_kv(map, key)      _Generic((key)+0, \
													int8_t : harbol_linkmap_index_get_kv, \
													uint8_t : harbol_linkmap_index_get_kv, \
													int16_t : harbol_linkmap_index_get_kv, \
													uint16_t : harbol_linkmap_index_get_kv, \
													int32_t : harbol_linkmap_index_get_kv, \
													uint32_t : harbol_linkmap_index_get_kv, \
													int64_t : harbol_linkmap_index_get_kv, \
													uint64_t : harbol_linkmap_index_get_kv, \
													char* : harbol_linkmap_key_get_kv, \
													const char* : harbol_linkmap_key_get_kv) \
												((map), (key))
											
#	define harbol_linkmap_set(map, key, val)    _Generic((key)+0, \
													int8_t : harbol_linkmap_index_set, \
													uint8_t : harbol_linkmap_index_set, \
													int16_t : harbol_linkmap_index_set, \
													uint16_t : harbol_linkmap_index_set, \
													int32_t : harbol_linkmap_index_set, \
													uint32_t : harbol_linkmap_index_set, \
													int64_t : harbol_linkmap_index_set, \
													uint64_t : harbol_linkmap_index_set, \
													char* : harbol_linkmap_key_set, \
													const char* : harbol_linkmap_key_set) \
												((map), (key), (val))
											
#	define harbol_linkmap_del(map, key, dtor)   _Generic((key), \
													int8_t : harbol_linkmap_index_del, \
													uint8_t : harbol_linkmap_index_del, \
													int16_t : harbol_linkmap_index_del, \
													uint16_t : harbol_linkmap_index_del, \
													int32_t : harbol_linkmap_index_del, \
													uint32_t : harbol_linkmap_index_del, \
													int64_t : harbol_linkmap_index_del, \
													uint64_t : harbol_linkmap_index_del, \
													char* : harbol_linkmap_key_del, \
													const char* : harbol_linkmap_key_del) \
												((map), (key), (dtor))
											
#	define harbol_linkmap_get_index(map, key)   _Generic((key), \
													int8_t : harbol_linkmap_get_val_index, \
													uint8_t : harbol_linkmap_get_val_index, \
													int16_t : harbol_linkmap_get_val_index, \
													uint16_t : harbol_linkmap_get_val_index, \
													int32_t : harbol_linkmap_get_val_index, \
													uint32_t : harbol_linkmap_get_val_index, \
													int64_t : harbol_linkmap_get_val_index, \
													uint64_t : harbol_linkmap_get_val_index, \
													struct HarbolKeyVal* : harbol_linkmap_get_node_index, \
													const struct HarbolKeyVal* : harbol_linkmap_get_node_index, \
													char* : harbol_linkmap_get_key_index, \
													const char* : harbol_linkmap_get_key_index) \
												((map), (key), (dtor))
#endif

HARBOL_EXPORT NO_NULL void *harbol_linkmap_get_iter(const struct HarbolLinkMap *map);
HARBOL_EXPORT NO_NULL void *harbol_linkmap_get_iter_end_count(const struct HarbolLinkMap *map);
HARBOL_EXPORT NO_NULL void *harbol_linkmap_get_iter_end_len(const struct HarbolLinkMap *map);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_LINKMAP_INCLUDED */
