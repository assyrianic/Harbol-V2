#include "tuple.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


typedef struct {
	uint16_t size;
	uint16_t offset;
} TupleElem_t;

HARBOL_EXPORT struct HarbolTuple *harbol_tuple_new(const size_t len, const size_t sizes[const static 1], const bool packed)
{
	struct HarbolTuple *tuple = harbol_alloc(1, sizeof *tuple);
	if( tuple != NULL )
		*tuple = harbol_tuple_create(len, sizes, packed);
	return tuple;
}

HARBOL_EXPORT struct HarbolTuple harbol_tuple_create(const size_t len, const size_t sizes[const static 1], const bool packed)
{
	struct HarbolTuple tuple = {harbol_vector_create(sizeof(TupleElem_t), 4), NULL, 0, packed};
	
	const size_t ptr_size = sizeof(intptr_t);
	size_t largest_memb = 0;
	
	// first we find the largest member of the tuple:
	for( uindex_t i=0; i<len; i++ )
		if( largest_memb<sizes[i] )
			largest_memb=sizes[i];
	
	// next, compute padding and alignment. we do this by having a next and previous size.
	size_t
		total_size=0,
		prev_size=0
	;
	for( uindex_t i=0; i<len; i++ ) {
		total_size += sizes[i];
		if( packed || len==1 )
			continue;
		else {
			const size_t offalign = (i+1 < len) ? sizes[i+1] : prev_size;
			total_size = harbol_align_size(total_size, offalign >= ptr_size ? ptr_size : offalign);
			prev_size = sizes[i];
		}
	}
	
	// now do a final size alignment with the largest member.
	const size_t aligned_total = harbol_align_size(total_size, largest_memb >= ptr_size ? ptr_size : largest_memb);
	tuple.datum = harbol_alloc(packed ? total_size : aligned_total, sizeof *tuple.datum);
	if( tuple.datum==NULL ) {
		harbol_vector_clear(&tuple.fields, NULL);
		return tuple;
	} else {
		tuple.len = packed ? total_size : aligned_total;
		uint32_t offset = 0;
		for( uindex_t i=0; i<len; i++ ) {
			harbol_vector_insert(&tuple.fields, &(TupleElem_t){sizes[i], offset});
			offset += sizes[i];
			if( packed || len==1 )
				continue;
			else {
				const size_t offalign = (i+1<len) ? sizes[i+1] : prev_size;
				offset = harbol_align_size(offset, offalign >= ptr_size ? ptr_size : offalign);
				prev_size = sizes[i];
			}
		}
		return tuple;
	}
}

HARBOL_EXPORT bool harbol_tuple_clear(struct HarbolTuple *const tuple)
{
	if( tuple->datum==NULL || tuple->len==0 )
		return false;
	else {
		harbol_vector_clear(&tuple->fields, NULL);
		harbol_free(tuple->datum), tuple->datum = NULL;
		tuple->len = 0;
		return true;
	}
}

HARBOL_EXPORT bool harbol_tuple_free(struct HarbolTuple **tupleref)
{
	if( *tupleref==NULL )
		return false;
	else {
		const bool res = harbol_tuple_clear(*tupleref);
		harbol_free(*tupleref), *tupleref=NULL;
		return res;
	}
}

HARBOL_EXPORT size_t harbol_tuple_len(const struct HarbolTuple *const tuple)
{
	return tuple->len;
}

HARBOL_EXPORT size_t harbol_tuple_fields(const struct HarbolTuple *const tuple)
{
	return tuple->fields.count;
}

HARBOL_EXPORT void *harbol_tuple_get(const struct HarbolTuple *const tuple, const uindex_t index)
{
	if( tuple->datum==NULL || tuple->len==0 )
		return NULL;
	else {
		const TupleElem_t *const field = harbol_vector_get(&tuple->fields, index);
		return( field==NULL || field->offset >= tuple->len ) ? NULL : tuple->datum + field->offset;
	}
}

HARBOL_EXPORT void *harbol_tuple_set(const struct HarbolTuple *const restrict tuple, const uindex_t index, void *const val)
{
	if( tuple->datum==NULL || tuple->len==0 )
		return NULL;
	else {
		void *field = harbol_tuple_get(tuple, index);
		if( field==NULL )
			return NULL;
		else {
			const TupleElem_t *const field_data = harbol_vector_get(&tuple->fields, index);
			memcpy(field, val, field_data->size);
			return field;
		}
	}
}

HARBOL_EXPORT size_t harbol_tuple_field_size(const struct HarbolTuple *const tuple, const uindex_t index)
{
	if( tuple->datum==NULL || tuple->len==0 )
		return 0;
	else {
		const TupleElem_t *const field_data = harbol_vector_get(&tuple->fields, index);
		return( field_data==NULL ) ? 0 : field_data->size;
	}
}

HARBOL_EXPORT bool harbol_tuple_packed(const struct HarbolTuple *const tuple)
{
	return tuple->packed;
}

HARBOL_EXPORT bool harbol_tuple_to_struct(const struct HarbolTuple *const restrict tuple, void *const struckt)
{
	return( tuple->datum==NULL || tuple->len==0 ) ? false : memcpy(struckt, tuple->datum, tuple->len) != NULL;
}
