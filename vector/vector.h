#ifndef HARBOL_VECTOR_INCLUDED
#	define HARBOL_VECTOR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"

#ifndef VEC_DEFAULT_SIZE
#	define VEC_DEFAULT_SIZE    4
#endif


struct HarbolVector {
	uint8_t *table;
	size_t len, count, datasize;
};

#define EMPTY_HARBOL_VECTOR    {NULL,0,0,0}


HARBOL_EXPORT struct HarbolVector *harbol_vector_new(size_t datasize, size_t default_size);
HARBOL_EXPORT struct HarbolVector harbol_vector_create(size_t datasize, size_t default_size);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_vector_clear(struct HarbolVector *vec, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_vector_free(struct HarbolVector **vecref, void dtor(void**));

HARBOL_EXPORT NO_NULL void *harbol_vector_get_iter(const struct HarbolVector *vec);
HARBOL_EXPORT NO_NULL void *harbol_vector_get_iter_end_len(const struct HarbolVector *vec);
HARBOL_EXPORT NO_NULL void *harbol_vector_get_iter_end_count(const struct HarbolVector *vec);

HARBOL_EXPORT NO_NULL bool harbol_vector_resize(struct HarbolVector *vec);
HARBOL_EXPORT NO_NULL bool harbol_vector_truncate(struct HarbolVector *vec);
HARBOL_EXPORT NO_NULL bool harbol_vector_reverse(struct HarbolVector *vec, void swap_fn(void *i, void *n));

HARBOL_EXPORT NO_NULL bool harbol_vector_insert(struct HarbolVector *vec, void *val);
HARBOL_EXPORT NO_NULL void *harbol_vector_pop(struct HarbolVector *vec);
HARBOL_EXPORT NO_NULL void *harbol_vector_get(const struct HarbolVector *vec, uindex_t index);
HARBOL_EXPORT NO_NULL bool harbol_vector_set(struct HarbolVector *vec, uindex_t index, void *val);

HARBOL_EXPORT NEVER_NULL(1) void harbol_vector_del(struct HarbolVector *vec, uindex_t index, void dtor(void**));
HARBOL_EXPORT NO_NULL void harbol_vector_add(struct HarbolVector *vecA, const struct HarbolVector *vecB);
HARBOL_EXPORT NO_NULL void harbol_vector_copy(struct HarbolVector *vecA, const struct HarbolVector *vecB);

HARBOL_EXPORT size_t harbol_vector_count_item(const struct HarbolVector *v, void *val);
HARBOL_EXPORT NO_NULL index_t harbol_vector_index_of(const struct HarbolVector *v, void *val, uindex_t starting_index);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_VECTOR_INCLUDED */
