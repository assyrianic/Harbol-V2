#include "map.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolKeyVal *harbol_kvpair_new(const char cstr[restrict static 1], void *const restrict data, const size_t datasize)
{
	struct HarbolKeyVal *kv = harbol_alloc(1, sizeof *kv);
	if( kv != NULL ) {
		kv->data = harbol_alloc(datasize, sizeof *kv->data);
		if( kv->data==NULL ) {
			harbol_free(kv), kv=NULL;
		} else {
			kv->key = harbol_string_create(cstr);
			if( kv->key.cstr==NULL )
				harbol_free(kv), kv=NULL;
			else memcpy(kv->data, data, datasize);
		}
	}
	return kv;
}

HARBOL_EXPORT bool harbol_kvpair_free(struct HarbolKeyVal **const kvpairref, void dtor(void**))
{
	harbol_string_clear(&(*kvpairref)->key);
	if( dtor != NULL )
		dtor((void**)&(*kvpairref)->data);
	if( (*kvpairref)->data != NULL )
		harbol_free((*kvpairref)->data), (*kvpairref)->data = NULL;
	harbol_free(*kvpairref), *kvpairref=NULL;
	return true;
}


HARBOL_EXPORT struct HarbolMap *harbol_map_new(const size_t datasize)
{
	struct HarbolMap *map = harbol_alloc(1, sizeof *map);
	if( map != NULL )
		*map = harbol_map_create(datasize);
	return map;
}

HARBOL_EXPORT struct HarbolMap harbol_map_create(const size_t datasize)
{
	struct HarbolMap map = {.datasize = datasize};
	return map;
}

HARBOL_EXPORT bool harbol_map_clear(struct HarbolMap *const map, void dtor(void**))
{
	if( map->buckets==NULL || map->datasize==0 )
		return false;
	else {
		for( uindex_t i=0; i<map->len; i++ ) {
			struct HarbolVector *const bucket = &map->buckets[i];
			for( uindex_t a=0; a<bucket->count; a++ )
				harbol_kvpair_free(harbol_vector_get(bucket, a), dtor);
			harbol_vector_clear(bucket, NULL);
		}
		harbol_free(map->buckets), map->buckets=NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_map_free(struct HarbolMap **mapref, void dtor(void**))
{
	if( *mapref==NULL )
		return false;
	else {
		const bool res = harbol_map_clear(*mapref, dtor);
		harbol_free(*mapref), *mapref=NULL;
		return res;
	}
}

HARBOL_EXPORT bool harbol_map_insert(struct HarbolMap *const restrict map, const char key[restrict static 1], void *restrict val)
{
	if( map->datasize==0 )
		return false;
	else {
		struct HarbolKeyVal *kv = harbol_kvpair_new(key, val, map->datasize);
		if( kv==NULL ) {
			return false;
		} else {
			const bool result = harbol_map_insert_kv(map, kv);
			if( !result ) {
				harbol_kvpair_free(&kv, NULL);
				return false;
			}
			else return true;
		}
	}
}

HARBOL_EXPORT bool harbol_map_insert_kv(struct HarbolMap *const map, struct HarbolKeyVal *kv)
{
	if( harbol_map_has_key(map, kv->key.cstr) )
		return false;
	else {
		if( map->len==0 || map->count >= map->len )
			harbol_map_rehash(map, (map->len==0) ? MAP_DEFAULT_SIZE : map->len << 1);
		
		const size_t hash = string_hash(kv->key.cstr) % map->len;
		struct HarbolVector *bucket = &map->buckets[hash];
		if( bucket->datasize==0 )
			bucket->datasize = sizeof(struct HarbolKeyVal *);
		harbol_vector_insert(bucket, &kv);
		map->count++;
		return true;
	}
}

HARBOL_EXPORT void *harbol_map_get(const struct HarbolMap *const restrict map, const char key[restrict static 1])
{
	if( map->buckets==NULL || !harbol_map_has_key(map, key) )
		return NULL;
	else {
		const size_t hash = string_hash(key) % map->len;
		struct HarbolVector *const bucket = &map->buckets[hash];
		for( uindex_t i=0; i<bucket->count; i++ ) {
			struct HarbolKeyVal **const kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->key, key) )
				return (*kv)->data;
		}
		return NULL;
	}
}

HARBOL_EXPORT struct HarbolKeyVal *harbol_map_get_kv(const struct HarbolMap *const restrict map, const char key[restrict static 1])
{
	if( map->buckets==NULL )
		return NULL;
	else {
		const size_t hash = string_hash(key) % map->len;
		struct HarbolVector *const bucket = &map->buckets[hash];
		for( uindex_t i=0; i<bucket->count; i++ ) {
			struct HarbolKeyVal **kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->key, key) )
				return *kv;
		}
		return NULL;
	}
}

HARBOL_EXPORT bool harbol_map_set(struct HarbolMap *const restrict map, const char key[restrict static 1], void *const restrict val)
{
	if( map->datasize==0 )
		return false;
	else if( map->buckets==NULL || !harbol_map_has_key(map, key) )
		return harbol_map_insert(map, key, val);
	else {
		const size_t hash = string_hash(key) % map->len;
		struct HarbolVector *const bucket = &map->buckets[hash];
		for( uindex_t i=0; i<bucket->count; i++ ) {
			struct HarbolKeyVal **const kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->key, key) ) {
				memcpy((*kv)->data, val, map->datasize);
				return true;
			}
		}
		return false;
	}
}

HARBOL_EXPORT bool harbol_map_has_key(const struct HarbolMap *const restrict map, const char key[restrict static 1])
{
	if( map->buckets==NULL )
		return false;
	else {
		const size_t hash = string_hash(key) % map->len;
		struct HarbolVector *const bucket = &map->buckets[hash];
		for( uindex_t i=0; i<bucket->count; i++ ) {
			struct HarbolKeyVal **const kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->key, key) )
				return true;
		}
		return false;
	}
}

HARBOL_EXPORT bool harbol_map_rehash(struct HarbolMap *const map, const size_t new_len)
{
	const size_t old_len = map->len;
	struct HarbolVector *curr = map->buckets;
	map->buckets = harbol_alloc(new_len, sizeof *map->buckets);
	if( map->buckets==NULL ) {
		map->buckets = curr;
		return false;
	} else {
		map->len = new_len;
		map->count = 0;
		if( curr != NULL ) {
			for( uindex_t i=0; i<old_len; i++ ) {
				struct HarbolVector *entry = &curr[i];
				for( uindex_t a=0; a<entry->count; a++ )
					harbol_map_insert_kv(map, *(struct HarbolKeyVal **)harbol_vector_get(entry, a));
				harbol_free(entry->table), entry->table = NULL;
			}
			harbol_free(curr), curr=NULL;
		}
		return true;
	}
}

HARBOL_EXPORT bool harbol_map_del(struct HarbolMap *const restrict map, const char key[restrict static 1], void dtor(void**))
{
	if( map->buckets==NULL || !harbol_map_has_key(map, key) )
		return false;
	else {
		const size_t hash = string_hash(key) % map->len;
		struct HarbolVector *const bucket = &map->buckets[hash];
		for( uindex_t i=0; i<bucket->count; i++ ) {
			struct HarbolKeyVal **const kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->key, key) ) {
				harbol_kvpair_free(kv, dtor);
				harbol_vector_del(bucket, i, NULL);
				map->count--;
				return true;
			}
		}
		return false;
	}
}
