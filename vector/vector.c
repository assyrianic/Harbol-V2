#include "vector.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

HARBOL_EXPORT struct HarbolVector *harbol_vector_new(const size_t datasize, const size_t default_size)
{
	struct HarbolVector *v = harbol_alloc(1, sizeof *v);
	if( v != NULL )
		*v = harbol_vector_create(datasize, default_size);
	return v;
}

HARBOL_EXPORT struct HarbolVector harbol_vector_create(const size_t datasize, const size_t default_size)
{
	struct HarbolVector vec = {NULL, 0, 0, datasize};
	harbol_generic_vector_resizer(&vec, default_size < VEC_DEFAULT_SIZE ? VEC_DEFAULT_SIZE : default_size, vec.datasize);
	return vec;
}

HARBOL_EXPORT bool harbol_vector_clear(struct HarbolVector *const v, void dtor(void**))
{
	if( dtor != NULL && v->datasize > 0 )
		for( uindex_t i=0; i<v->len; i++ )
			dtor((void**)&(uint8_t *){&v->table[i * v->datasize]});
	
	harbol_free(v->table), v->table = NULL;
	v->len = v->count = 0;
	return true;
}

HARBOL_EXPORT bool harbol_vector_free(struct HarbolVector **const vecref, void dtor(void**))
{
	if( *vecref==NULL )
		return false;
	else {
		const bool res = harbol_vector_clear(*vecref, dtor);
		harbol_free(*vecref), *vecref=NULL;
		return res;
	}
}

HARBOL_EXPORT void *harbol_vector_get_iter(const struct HarbolVector *const v)
{
	return v->table;
}

HARBOL_EXPORT void *harbol_vector_get_iter_end_len(const struct HarbolVector *const v)
{
	return v->table != NULL && v->datasize != 0 ? &v->table[v->len * v->datasize] : NULL;
}

HARBOL_EXPORT void *harbol_vector_get_iter_end_count(const struct HarbolVector *const v)
{
	return v->table != NULL && v->datasize != 0 ? &v->table[v->count * v->datasize] : NULL;
}

HARBOL_EXPORT bool harbol_vector_resize(struct HarbolVector *const v)
{
	if( v->datasize==0 )
		return false;
	else {
		const size_t old_len = v->len;
		harbol_generic_vector_resizer(v, v->len==0 ? VEC_DEFAULT_SIZE : v->len << 1, v->datasize);
		return v->len > old_len;
	}
}

HARBOL_EXPORT bool harbol_vector_truncate(struct HarbolVector *const v)
{
	if( v->datasize==0 || v->len==VEC_DEFAULT_SIZE )
		return false;
	else if( v->count < (v->len >> 1) ) {
		const size_t old_len = v->len;
		harbol_generic_vector_resizer(v, v->len >> 1, v->datasize);
		return old_len > v->len;
	}
	else return false;
}

HARBOL_EXPORT bool harbol_vector_reverse(struct HarbolVector *const v, void swap_fn(void *i, void *n))
{
	if( v->table==NULL || v->datasize==0 )
		return false;
	else {
		for( uindex_t i=0, n=v->count-1; i<v->count/2; i++, n-- )
			swap_fn(&v->table[i * v->datasize], &v->table[n * v->datasize]);
		return true;
	}
}

HARBOL_EXPORT bool harbol_vector_insert(struct HarbolVector *const restrict v, void *restrict val)
{
	if( v->datasize==0 )
		return false;
	else {
		if( v->table==NULL || v->count >= v->len )
			harbol_vector_resize(v);
		
		memcpy(&v->table[v->count * v->datasize], val, v->datasize);
		v->count++;
		return true;
	}
}

HARBOL_EXPORT void *harbol_vector_pop(struct HarbolVector *const v)
{
	return( v->table==NULL || v->count==0  || v->datasize==0 ) ? NULL : &v->table[--v->count * v->datasize];
}

HARBOL_EXPORT void *harbol_vector_get(const struct HarbolVector *const v, const uindex_t index)
{
	return( v->table==NULL || v->datasize==0 || index >= v->count ) ? NULL : &v->table[index * v->datasize];
}

HARBOL_EXPORT bool harbol_vector_set(struct HarbolVector *const restrict v, const uindex_t index, void *restrict val)
{
	if( v->datasize==0 || index >= v->count )
		return false;
	else if( v->table==NULL )
		return harbol_vector_insert(v, val);
	else return memcpy(&v->table[index * v->datasize], val, v->datasize) != NULL;
}

HARBOL_EXPORT void harbol_vector_del(struct HarbolVector *const v, const uindex_t index, void dtor(void**))
{
	if( v->table==NULL || v->datasize==0 || index >= v->count )
		return;
	else {
		if( dtor != NULL )
			dtor((void**)&(uint8_t *){&v->table[index * v->datasize]});
		
		const uindex_t
			i=index+1,
			j=index
		;
		v->count--;
		memmove(&v->table[j * v->datasize], &v->table[i * v->datasize], (v->count - j) * v->datasize);
		memset(&v->table[v->count * v->datasize], 0, v->datasize);
	}
}

HARBOL_EXPORT void harbol_vector_add(struct HarbolVector *const vA, const struct HarbolVector *const vB)
{
	if( vB->table==NULL || vB->datasize==0 || vA->datasize != vB->datasize )
		return;
	else {
		if( !vA->table || vA->count + vB->count >= vA->len )
			while( (vA->count + vB->count) >= vA->len )
				harbol_vector_resize(vA);
		memcpy(&vA->table[vA->count * vA->datasize], vB->table, vB->count * vB->datasize);
		vA->count += vB->count;
	}
}

HARBOL_EXPORT void harbol_vector_copy(struct HarbolVector *const vA, const struct HarbolVector *const vB)
{
	if( vB->table==NULL || !vB->datasize )
		return;
	else {
		harbol_vector_clear(vA, NULL);
		vA->datasize = vB->datasize;
		if( vB->count >= vA->len )
			while( vB->count >= vA->len )
				harbol_vector_resize(vA);
		
		memcpy(vA->table, vB->table, vB->count * vB->datasize);
		vA->count = vB->count;
	}
}

HARBOL_EXPORT size_t harbol_vector_count_item(const struct HarbolVector *const restrict v, void *const restrict val)
{
	if( v->table==NULL || v->datasize==0 )
		return 0;
	else {
		size_t occurrences = 0;
		for( uindex_t i=0; i<v->count; i++ )
			if( !memcmp(&v->table[i * v->datasize], val, v->datasize) )
				occurrences++;
		return occurrences;
	}
}

HARBOL_EXPORT index_t harbol_vector_index_of(const struct HarbolVector *const restrict v, void *const restrict val, const uindex_t starting_index)
{
	if( v->table==NULL || v->datasize==0 )
		return -1;
	else {
		if( starting_index >= v->count )
			return -1;
		else {
			for( uindex_t i=starting_index; i<v->count; i++ )
				if( !memcmp(&v->table[i * v->datasize], val, v->datasize) )
					return i;
			return -1;
		}
	}
}
