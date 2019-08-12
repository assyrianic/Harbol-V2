#include "linkmap.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolLinkMap *harbol_linkmap_new(const size_t datasize)
{
	struct HarbolLinkMap *map = harbol_alloc(1, sizeof *map);
	if( map != NULL )
		*map = harbol_linkmap_create(datasize);
	return map;
}

HARBOL_EXPORT struct HarbolLinkMap harbol_linkmap_create(const size_t datasize)
{
	struct HarbolLinkMap map = { harbol_map_create(datasize), harbol_vector_create(sizeof(struct HarbolKeyVal *), MAP_DEFAULT_SIZE)};
	return map;
}

HARBOL_EXPORT bool harbol_linkmap_clear(struct HarbolLinkMap *const map, void dtor(void**))
{
	harbol_map_clear(&map->map, dtor);
	harbol_vector_clear(&map->vec, NULL);
	return true;
}

HARBOL_EXPORT bool harbol_linkmap_free(struct HarbolLinkMap **const mapref, void dtor(void**))
{
	harbol_linkmap_clear(*mapref, dtor);
	harbol_free(*mapref), *mapref=NULL;
	return true;
}

HARBOL_EXPORT size_t harbol_linkmap_count(const struct HarbolLinkMap *const map)
{
	return map->vec.count;
}

HARBOL_EXPORT bool harbol_linkmap_has_key(const struct HarbolLinkMap *const restrict map, const char key[restrict static 1])
{
	return harbol_map_has_key(&map->map, key);
}

HARBOL_EXPORT bool harbol_linkmap_insert(struct HarbolLinkMap *const restrict map, const char key[restrict static 1], void *const restrict val)
{
	harbol_map_insert(&map->map, key, val);
	struct HarbolKeyVal *kv = harbol_map_get_kv(&map->map, key);
	harbol_vector_insert(&map->vec, &kv);
	return true;
}

HARBOL_EXPORT bool harbol_linkmap_insert_kv(struct HarbolLinkMap *const map, struct HarbolKeyVal *kv)
{
	harbol_map_insert_kv(&map->map, kv);
	harbol_vector_insert(&map->vec, &kv);
	return true;
}


HARBOL_EXPORT void *harbol_linkmap_key_get(const struct HarbolLinkMap *const restrict map, const char key[restrict static 1])
{
	return harbol_map_get(&map->map, key);
}

HARBOL_EXPORT void *harbol_linkmap_index_get(const struct HarbolLinkMap *const map, const uindex_t index)
{
	struct HarbolKeyVal **kv = harbol_vector_get(&map->vec, index);
	return( kv==NULL ) ? NULL : (*kv)->data;
}


HARBOL_EXPORT struct HarbolKeyVal *harbol_linkmap_key_get_kv(const struct HarbolLinkMap *const map, const char key[restrict static 1])
{
	return harbol_map_get_kv(&map->map, key);
}

HARBOL_EXPORT struct HarbolKeyVal *harbol_linkmap_index_get_kv(const struct HarbolLinkMap *const map, const uindex_t index)
{
	struct HarbolKeyVal **kv = harbol_vector_get(&map->vec, index);
	return( kv==NULL ) ? NULL : *kv;
}


HARBOL_EXPORT bool harbol_linkmap_key_set(struct HarbolLinkMap *const map, const char key[restrict static 1], void *const restrict val)
{
	return harbol_map_set(&map->map, key, val);
}

HARBOL_EXPORT bool harbol_linkmap_index_set(struct HarbolLinkMap *const map, const uindex_t index, void *const restrict val)
{
	struct HarbolKeyVal **kv = harbol_vector_get(&map->vec, index);
	return( kv==NULL || map->map.datasize==0 ) ? false : memcpy((*kv)->data, val, map->map.datasize) != NULL;
}

HARBOL_EXPORT bool harbol_linkmap_key_del(struct HarbolLinkMap *const map, const char key[restrict static 1], void dtor(void**))
{
	struct HarbolKeyVal *kv = harbol_map_get_kv(&map->map, key);
	harbol_map_del(&map->map, key, dtor);
	harbol_vector_del(&map->vec, harbol_vector_index_of(&map->vec, &kv, 0), NULL);
	return true;
}

HARBOL_EXPORT bool harbol_linkmap_index_del(struct HarbolLinkMap *const map, const uindex_t index, void dtor(void**))
{
	struct HarbolKeyVal **kv = harbol_vector_get(&map->vec, index);
	if( !kv )
		return false;
	else {
		harbol_map_del(&map->map, (*kv)->key.cstr, dtor);
		harbol_vector_del(&map->vec, index, NULL);
		return true;
	}
}

HARBOL_EXPORT index_t harbol_linkmap_get_key_index(const struct HarbolLinkMap *const map, const char key[restrict static 1])
{
	if( map->vec.datasize==0 )
		return -1;
	else {
		for( uindex_t i=0; i<map->vec.count; i++ ) {
			struct HarbolKeyVal **kv = harbol_vector_get(&map->vec, i);
			if( !harbol_string_cmpcstr(&(*kv)->key, key) )
				return i;
		}
		return -1;
	}
}

HARBOL_EXPORT index_t harbol_linkmap_get_node_index(const struct HarbolLinkMap *const map, struct HarbolKeyVal *findkv)
{
	return harbol_vector_index_of(&map->vec, &findkv, 0);
}

HARBOL_EXPORT index_t harbol_linkmap_get_val_index(const struct HarbolLinkMap *const map, void *const restrict val)
{
	if( map->map.datasize==0 )
		return -1;
	else {
		for( uindex_t i=0; i<map->vec.count; i++ ) {
			struct HarbolKeyVal **kv = harbol_vector_get(&map->vec, i);
			if( !memcmp((*kv)->data, val, map->map.datasize) )
				return i;
		}
		return -1;
	}
}

HARBOL_EXPORT void *harbol_linkmap_get_iter(const struct HarbolLinkMap *const map)
{
	return harbol_vector_get_iter(&map->vec);
}

HARBOL_EXPORT void *harbol_linkmap_get_iter_end_count(const struct HarbolLinkMap *const map)
{
	return harbol_vector_get_iter_end_count(&map->vec);
}

HARBOL_EXPORT void *harbol_linkmap_get_iter_end_len(const struct HarbolLinkMap *const map)
{
	return harbol_vector_get_iter_end_len(&map->vec);
}
