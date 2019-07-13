#include "vector.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

HARBOL_EXPORT struct HarbolVector *harbol_vector_new(const size_t datasize, const size_t default_size)
{
	struct HarbolVector *v = calloc(1, sizeof *v);
	if( v != NULL )
		*v = harbol_vector_create(datasize, default_size);
	return v;
}

HARBOL_EXPORT struct HarbolVector harbol_vector_create(const size_t datasize, const size_t default_size)
{
	struct HarbolVector vec = {NULL, 0, 0, datasize};
	harbol_generic_vector_resizer(&vec, default_size < VEC_DEFAULT_SIZE ? VEC_DEFAULT_SIZE : default_size, vec.DataSize);
	return vec;
}

HARBOL_EXPORT bool harbol_vector_clear(struct HarbolVector *const v, void dtor(void**))
{
	if( dtor != NULL && v->DataSize > 0 )
		for( uindex_t i=0; i<v->Len; i++ )
			dtor((void**)&(uint8_t *){&v->Table[i * v->DataSize]});
	
	free(v->Table), v->Table = NULL;
	v->Len = v->Count = 0;
	return true;
}

HARBOL_EXPORT bool harbol_vector_free(struct HarbolVector **const vecref, void dtor(void**))
{
	if( *vecref==NULL )
		return false;
	else {
		const bool res = harbol_vector_clear(*vecref, dtor);
		free(*vecref), *vecref=NULL;
		return res;
	}
}

HARBOL_EXPORT void *harbol_vector_get_iter(const struct HarbolVector *const v)
{
	return v->Table;
}

HARBOL_EXPORT void *harbol_vector_get_iter_end_len(const struct HarbolVector *const v)
{
	return v->Table != NULL && v->DataSize != 0 ? &v->Table[v->Len * v->DataSize] : NULL;
}

HARBOL_EXPORT void *harbol_vector_get_iter_end_count(const struct HarbolVector *const v)
{
	return v->Table != NULL && v->DataSize != 0 ? &v->Table[v->Count * v->DataSize] : NULL;
}

HARBOL_EXPORT bool harbol_vector_resize(struct HarbolVector *const v)
{
	if( v->DataSize==0 )
		return false;
	else {
		const size_t old_len = v->Len;
		harbol_generic_vector_resizer(v, v->Len==0 ? VEC_DEFAULT_SIZE : v->Len << 1, v->DataSize);
		return v->Len > old_len;
	}
}

HARBOL_EXPORT bool harbol_vector_truncate(struct HarbolVector *const v)
{
	if( v->DataSize==0 || v->Len==VEC_DEFAULT_SIZE )
		return false;
	else if( v->Count < (v->Len >> 1) ) {
		const size_t old_len = v->Len;
		harbol_generic_vector_resizer(v, v->Len >> 1, v->DataSize);
		return old_len > v->Len;
	}
	else return false;
}

HARBOL_EXPORT bool harbol_vector_reverse(struct HarbolVector *const v, void swap_fn(void *i, void *n))
{
	if( v->Table==NULL || v->DataSize==0 )
		return false;
	else {
		for( uindex_t i=0, n=v->Count-1; i<v->Count/2; i++, n-- )
			swap_fn(&v->Table[i * v->DataSize], &v->Table[n * v->DataSize]);
		return true;
	}
}

HARBOL_EXPORT bool harbol_vector_insert(struct HarbolVector *const restrict v, void *restrict val)
{
	if( v->DataSize==0 )
		return false;
	else {
		if( v->Table==NULL || v->Count >= v->Len )
			harbol_vector_resize(v);
		
		memcpy(&v->Table[v->Count * v->DataSize], val, v->DataSize);
		v->Count++;
		return true;
	}
}

HARBOL_EXPORT void *harbol_vector_pop(struct HarbolVector *const v)
{
	return( v->Table==NULL || v->Count==0  || v->DataSize==0 ) ? NULL : &v->Table[--v->Count * v->DataSize];
}

HARBOL_EXPORT void *harbol_vector_get(const struct HarbolVector *const v, const uindex_t index)
{
	return( v->Table==NULL || v->DataSize==0 || index >= v->Count ) ? NULL : &v->Table[index * v->DataSize];
}

HARBOL_EXPORT bool harbol_vector_set(struct HarbolVector *const restrict v, const uindex_t index, void *restrict val)
{
	if( v->DataSize==0 || index >= v->Count )
		return false;
	else if( v->Table==NULL )
		return harbol_vector_insert(v, val);
	else return memcpy(&v->Table[index * v->DataSize], val, v->DataSize) != NULL;
}

HARBOL_EXPORT void harbol_vector_del(struct HarbolVector *const v, const uindex_t index, void dtor(void**))
{
	if( v->Table==NULL || v->DataSize==0 || index >= v->Count )
		return;
	else {
		if( dtor != NULL )
			dtor((void**)&(uint8_t *){&v->Table[index * v->DataSize]});
		
		const uindex_t
			i=index+1,
			j=index
		;
		v->Count--;
		memmove(&v->Table[j * v->DataSize], &v->Table[i * v->DataSize], (v->Count - j) * v->DataSize);
	}
}

HARBOL_EXPORT void harbol_vector_add(struct HarbolVector *const vA, const struct HarbolVector *const vB)
{
	if( vB->Table==NULL || vB->DataSize==0 || vA->DataSize != vB->DataSize )
		return;
	else {
		if( !vA->Table || vA->Count + vB->Count >= vA->Len )
			while( (vA->Count + vB->Count) >= vA->Len )
				harbol_vector_resize(vA);
		memcpy(&vA->Table[vA->Count * vA->DataSize], vB->Table, vB->Count * vB->DataSize);
		vA->Count += vB->Count;
	}
}

HARBOL_EXPORT void harbol_vector_copy(struct HarbolVector *const vA, const struct HarbolVector *const vB)
{
	if( vB->Table==NULL || !vB->DataSize )
		return;
	else {
		harbol_vector_clear(vA, NULL);
		vA->DataSize = vB->DataSize;
		if( vB->Count >= vA->Len )
			while( vB->Count >= vA->Len )
				harbol_vector_resize(vA);
		
		memcpy(vA->Table, vB->Table, vB->Count * vB->DataSize);
		vA->Count = vB->Count;
	}
}

HARBOL_EXPORT size_t harbol_vector_count_item(const struct HarbolVector *const restrict v, void *const restrict val)
{
	if( v->Table==NULL || v->DataSize==0 )
		return 0;
	else {
		size_t occurrences = 0;
		for( uindex_t i=0; i<v->Count; i++ )
			if( !memcmp(&v->Table[i * v->DataSize], val, v->DataSize) )
				occurrences++;
		return occurrences;
	}
}

HARBOL_EXPORT index_t harbol_vector_index_of(const struct HarbolVector *v, void *const restrict val, const uindex_t starting_index)
{
	if( v->Table==NULL || v->DataSize==0 )
		return -1;
	else {
		if( starting_index >= v->Count )
			return -1;
		else {
			for( uindex_t i=starting_index; i<v->Count; i++ )
				if( !memcmp(&v->Table[i * v->DataSize], val, v->DataSize) )
					return i;
			return -1;
		}
	}
}
