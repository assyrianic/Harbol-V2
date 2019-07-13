#ifndef HARBOL_TUPLE_INCLUDED
#	define HARBOL_TUPLE_INCLUDED

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"
#include "../vector/vector.h"

#ifdef __cplusplus
extern "C" {
#endif


struct HarbolTuple {
	struct HarbolVector Fields;
	uint8_t *Datum;
	size_t Len;
	bool Packed : 1;
};

HARBOL_EXPORT NO_NULL struct HarbolTuple *harbol_tuple_new(size_t len, const size_t sizes[], bool packed);
HARBOL_EXPORT NO_NULL struct HarbolTuple harbol_tuple_create(size_t len, const size_t sizes[], bool packed);
HARBOL_EXPORT NO_NULL bool harbol_tuple_clear(struct HarbolTuple *tuple);
HARBOL_EXPORT NO_NULL bool harbol_tuple_free(struct HarbolTuple **tupleref);

HARBOL_EXPORT NO_NULL size_t harbol_tuple_len(const struct HarbolTuple *tuple);
HARBOL_EXPORT NO_NULL size_t harbol_tuple_fields(const struct HarbolTuple *tuple);

HARBOL_EXPORT NO_NULL void *harbol_tuple_get(const struct HarbolTuple *tuple, uindex_t index);
HARBOL_EXPORT NO_NULL void *harbol_tuple_set(const struct HarbolTuple *tuple, uindex_t index, void *field);

HARBOL_EXPORT NO_NULL size_t harbol_tuple_field_size(const struct HarbolTuple *tuple, uindex_t index);
HARBOL_EXPORT NO_NULL bool harbol_tuple_packed(const struct HarbolTuple *tuple);
HARBOL_EXPORT NO_NULL bool harbol_tuple_to_struct(const struct HarbolTuple *tuple, void *struckt);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_TUPLE_INCLUDED */
