#include "map.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolKeyVal *harbol_kvpair_new(const char cstr[restrict static 1], void *const restrict data, const size_t datasize)
{
	struct HarbolKeyVal *kv = calloc(1, sizeof *kv);
	if( kv != NULL ) {
		kv->Data = calloc(datasize, sizeof *kv->Data);
		if( kv->Data==NULL ) {
			free(kv), kv=NULL;
		} else {
			kv->Key = harbol_string_create(cstr);
			if( kv->Key.CStr==NULL )
				free(kv), kv=NULL;
			else memcpy(kv->Data, data, datasize);
		}
	}
	return kv;
}

HARBOL_EXPORT bool harbol_kvpair_free(struct HarbolKeyVal **const kvpairref, void dtor(void**))
{
	harbol_string_clear(&(*kvpairref)->Key);
	if( dtor != NULL )
		dtor((void**)&(*kvpairref)->Data);
	if( (*kvpairref)->Data != NULL )
		free((*kvpairref)->Data), (*kvpairref)->Data = NULL;
	free(*kvpairref), *kvpairref=NULL;
	return true;
}


HARBOL_EXPORT struct HarbolMap *harbol_map_new(const size_t datasize)
{
	struct HarbolMap *map = calloc(1, sizeof *map);
	if( map != NULL )
		*map = harbol_map_create(datasize);
	return map;
}

HARBOL_EXPORT struct HarbolMap harbol_map_create(const size_t datasize)
{
	struct HarbolMap map = {NULL, 0, 0, datasize};
	return map;
}

HARBOL_EXPORT bool harbol_map_clear(struct HarbolMap *const map, void dtor(void**))
{
	if( map->Buckets==NULL || map->DataSize==0 )
		return false;
	else {
		for( uindex_t i=0; i<map->Len; i++ ) {
			struct HarbolVector *const bucket = &map->Buckets[i];
			for( uindex_t a=0; a<bucket->Count; a++ )
				harbol_kvpair_free(harbol_vector_get(bucket, a), dtor);
			harbol_vector_clear(bucket, NULL);
		}
		free(map->Buckets), map->Buckets=NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_map_free(struct HarbolMap **mapref, void dtor(void**))
{
	if( *mapref==NULL )
		return false;
	else {
		const bool res = harbol_map_clear(*mapref, dtor);
		free(*mapref), *mapref=NULL;
		return res;
	}
}

HARBOL_EXPORT bool harbol_map_insert(struct HarbolMap *const restrict map, const char key[restrict static 1], void *restrict val)
{
	if( map->DataSize==0 )
		return false;
	else {
		struct HarbolKeyVal *kv = harbol_kvpair_new(key, val, map->DataSize);
		if( kv==NULL ) {
			return false;
		} else {
			const bool b = harbol_map_insert_kv(map, kv);
			if( !b ) {
				harbol_kvpair_free(&kv, NULL);
				return false;
			}
			else return true;
		}
	}
}

HARBOL_EXPORT bool harbol_map_insert_kv(struct HarbolMap *const map, struct HarbolKeyVal *kv)
{
	if( harbol_map_has_key(map, kv->Key.CStr) )
		return false;
	else {
		if( map->Len==0 || map->Count >= map->Len )
			harbol_map_rehash(map, (map->Len==0) ? MAP_DEFAULT_SIZE : map->Len << 1);
		
		const size_t hash = string_hash(kv->Key.CStr) % map->Len;
		struct HarbolVector *bucket = &map->Buckets[hash];
		if( bucket->DataSize==0 )
			bucket->DataSize = sizeof(struct HarbolKeyVal *);
		harbol_vector_insert(bucket, &kv);
		map->Count++;
		return true;
	}
}

HARBOL_EXPORT void *harbol_map_get(const struct HarbolMap *const restrict map, const char key[restrict static 1])
{
	if( map->Buckets==NULL || !harbol_map_has_key(map, key) )
		return NULL;
	else {
		const size_t hash = string_hash(key) % map->Len;
		struct HarbolVector *const bucket = &map->Buckets[hash];
		for( uindex_t i=0; i<bucket->Count; i++ ) {
			struct HarbolKeyVal **const kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->Key, key) )
				return (*kv)->Data;
		}
		return NULL;
	}
}

HARBOL_EXPORT struct HarbolKeyVal *harbol_map_get_kv(const struct HarbolMap *const restrict map, const char key[restrict static 1])
{
	if( map->Buckets==NULL )
		return NULL;
	else {
		const size_t hash = string_hash(key) % map->Len;
		struct HarbolVector *const bucket = &map->Buckets[hash];
		for( uindex_t i=0; i<bucket->Count; i++ ) {
			struct HarbolKeyVal **kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->Key, key) )
				return *kv;
		}
		return NULL;
	}
}

HARBOL_EXPORT bool harbol_map_set(struct HarbolMap *const restrict map, const char key[restrict static 1], void *const restrict val)
{
	if( map->DataSize==0 )
		return false;
	else if( map->Buckets==NULL || !harbol_map_has_key(map, key) )
		return harbol_map_insert(map, key, val);
	else {
		const size_t hash = string_hash(key) % map->Len;
		struct HarbolVector *const bucket = &map->Buckets[hash];
		for( uindex_t i=0; i<bucket->Count; i++ ) {
			struct HarbolKeyVal **const kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->Key, key) ) {
				memcpy((*kv)->Data, val, map->DataSize);
				return true;
			}
		}
		return false;
	}
}

HARBOL_EXPORT bool harbol_map_has_key(const struct HarbolMap *const restrict map, const char key[restrict static 1])
{
	if( map->Buckets==NULL )
		return false;
	else {
		const size_t hash = string_hash(key) % map->Len;
		struct HarbolVector *const bucket = &map->Buckets[hash];
		for( uindex_t i=0; i<bucket->Count; i++ ) {
			struct HarbolKeyVal **const kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->Key, key) )
				return true;
		}
		return false;
	}
}

HARBOL_EXPORT bool harbol_map_rehash(struct HarbolMap *const map, const size_t new_len)
{
	const size_t old_len = map->Len;
	struct HarbolVector *curr = map->Buckets;
	map->Buckets = calloc(new_len, sizeof *map->Buckets);
	if( map->Buckets==NULL ) {
		map->Buckets = curr;
		return false;
	} else {
		map->Len = new_len;
		map->Count = 0;
		if( curr != NULL ) {
			for( uindex_t i=0; i<old_len; i++ ) {
				struct HarbolVector *entry = &curr[i];
				for( uindex_t a=0; a<entry->Count; a++ )
					harbol_map_insert_kv(map, *(struct HarbolKeyVal **)harbol_vector_get(entry, a));
				free(entry->Table), entry->Table = NULL;
			}
			free(curr), curr=NULL;
		}
		return true;
	}
}

HARBOL_EXPORT bool harbol_map_del(struct HarbolMap *const restrict map, const char key[restrict static 1], void dtor(void**))
{
	if( map->Buckets==NULL || !harbol_map_has_key(map, key) )
		return false;
	else {
		const size_t hash = string_hash(key) % map->Len;
		struct HarbolVector *const bucket = &map->Buckets[hash];
		for( uindex_t i=0; i<bucket->Count; i++ ) {
			struct HarbolKeyVal **const kv = harbol_vector_get(bucket, i);
			if( !harbol_string_cmpcstr(&(*kv)->Key, key) ) {
				harbol_kvpair_free(kv, dtor);
				harbol_vector_del(bucket, i, NULL);
				map->Count--;
				return true;
			}
		}
		return false;
	}
}
