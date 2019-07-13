#include "linkmap.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolLinkMap *harbol_linkmap_new(const size_t datasize)
{
	struct HarbolLinkMap *map = calloc(1, sizeof *map);
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
	harbol_map_clear(&map->Map, dtor);
	harbol_vector_clear(&map->Vec, NULL);
	return true;
}

HARBOL_EXPORT bool harbol_linkmap_free(struct HarbolLinkMap **const mapref, void dtor(void**))
{
	harbol_linkmap_clear(*mapref, dtor);
	free(*mapref), *mapref=NULL;
	return true;
}

HARBOL_EXPORT size_t harbol_linkmap_count(const struct HarbolLinkMap *const map)
{
	return map->Vec.Count;
}

HARBOL_EXPORT bool harbol_linkmap_has_key(const struct HarbolLinkMap *const restrict map, const char key[restrict static 1])
{
	return harbol_map_has_key(&map->Map, key);
}

HARBOL_EXPORT bool harbol_linkmap_insert(struct HarbolLinkMap *const restrict map, const char key[restrict static 1], void *const restrict val)
{
	harbol_map_insert(&map->Map, key, val);
	struct HarbolKeyVal *kv = harbol_map_get_kv(&map->Map, key);
	harbol_vector_insert(&map->Vec, &kv);
	return true;
}

HARBOL_EXPORT bool harbol_linkmap_insert_kv(struct HarbolLinkMap *const map, struct HarbolKeyVal *kv)
{
	harbol_map_insert_kv(&map->Map, kv);
	harbol_vector_insert(&map->Vec, &kv);
	return true;
}


HARBOL_EXPORT void *harbol_linkmap_key_get(const struct HarbolLinkMap *const restrict map, const char key[restrict static 1])
{
	return harbol_map_get(&map->Map, key);
}

HARBOL_EXPORT void *harbol_linkmap_index_get(const struct HarbolLinkMap *const map, const uindex_t index)
{
	struct HarbolKeyVal **kv = harbol_vector_get(&map->Vec, index);
	return( kv==NULL ) ? NULL : (*kv)->Data;
}


HARBOL_EXPORT struct HarbolKeyVal *harbol_linkmap_key_get_kv(const struct HarbolLinkMap *const map, const char key[restrict static 1])
{
	return harbol_map_get_kv(&map->Map, key);
}

HARBOL_EXPORT struct HarbolKeyVal *harbol_linkmap_index_get_kv(const struct HarbolLinkMap *const map, const uindex_t index)
{
	struct HarbolKeyVal **kv = harbol_vector_get(&map->Vec, index);
	return( kv==NULL ) ? NULL : *kv;
}


HARBOL_EXPORT bool harbol_linkmap_key_set(struct HarbolLinkMap *const map, const char key[restrict static 1], void *const restrict val)
{
	return harbol_map_set(&map->Map, key, val);
}

HARBOL_EXPORT bool harbol_linkmap_index_set(struct HarbolLinkMap *const map, const uindex_t index, void *const restrict val)
{
	struct HarbolKeyVal **kv = harbol_vector_get(&map->Vec, index);
	return( kv==NULL || map->Map.DataSize==0 ) ? false : memcpy((*kv)->Data, val, map->Map.DataSize) != NULL;
}

HARBOL_EXPORT bool harbol_linkmap_key_del(struct HarbolLinkMap *const map, const char key[restrict static 1], void dtor(void**))
{
	struct HarbolKeyVal *kv = harbol_map_get_kv(&map->Map, key);
	harbol_map_del(&map->Map, key, dtor);
	harbol_vector_del(&map->Vec, harbol_vector_index_of(&map->Vec, &kv, 0), NULL);
	return true;
}

HARBOL_EXPORT bool harbol_linkmap_index_del(struct HarbolLinkMap *const map, const uindex_t index, void dtor(void**))
{
	struct HarbolKeyVal **kv = harbol_vector_get(&map->Vec, index);
	if( !kv )
		return false;
	else {
		harbol_map_del(&map->Map, (*kv)->Key.CStr, dtor);
		harbol_vector_del(&map->Vec, index, NULL);
		return true;
	}
}

HARBOL_EXPORT index_t harbol_linkmap_get_key_index(const struct HarbolLinkMap *const map, const char key[restrict static 1])
{
	if( map->Vec.DataSize==0 )
		return -1;
	else {
		for( uindex_t i=0; i<map->Vec.Count; i++ ) {
			struct HarbolKeyVal **kv = harbol_vector_get(&map->Vec, i);
			if( !harbol_string_cmpcstr(&(*kv)->Key, key) )
				return i;
		}
		return -1;
	}
}

HARBOL_EXPORT index_t harbol_linkmap_get_node_index(const struct HarbolLinkMap *const map, struct HarbolKeyVal *findkv)
{
	return harbol_vector_index_of(&map->Vec, &findkv, 0);
}

HARBOL_EXPORT index_t harbol_linkmap_get_val_index(const struct HarbolLinkMap *const map, void *const restrict val)
{
	if( map->Map.DataSize==0 )
		return -1;
	else {
		for( uindex_t i=0; i<map->Vec.Count; i++ ) {
			struct HarbolKeyVal **kv = harbol_vector_get(&map->Vec, i);
			if( !memcmp((*kv)->Data, val, map->Map.DataSize) )
				return i;
		}
		return -1;
	}
}

HARBOL_EXPORT void *harbol_linkmap_get_iter(const struct HarbolLinkMap *const map)
{
	return harbol_vector_get_iter(&map->Vec);
}

HARBOL_EXPORT void *harbol_linkmap_get_iter_end_count(const struct HarbolLinkMap *const map)
{
	return harbol_vector_get_iter_end_count(&map->Vec);
}

HARBOL_EXPORT void *harbol_linkmap_get_iter_end_len(const struct HarbolLinkMap *const map)
{
	return harbol_vector_get_iter_end_len(&map->Vec);
}
