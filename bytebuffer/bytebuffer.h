#ifndef HARBOL_BYTEBUFFER_INCLUDED
#	define HARBOL_BYTEBUFFER_INCLUDED

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"

#ifdef __cplusplus
extern "C" {
#endif


struct HarbolByteBuf {
	uint8_t *Table;
	size_t Len, Count;
};


HARBOL_EXPORT struct HarbolByteBuf *harbol_bytebuffer_new(void);
HARBOL_EXPORT struct HarbolByteBuf harbol_bytebuffer_create(void);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_clear(struct HarbolByteBuf *buf);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_free(struct HarbolByteBuf **bufref);

HARBOL_EXPORT NO_NULL size_t harbol_bytebuffer_len(const struct HarbolByteBuf *buf);
HARBOL_EXPORT NO_NULL size_t harbol_bytebuffer_count(const struct HarbolByteBuf *buf);
HARBOL_EXPORT NO_NULL uint8_t *harbol_bytebuffer_get_buffer(const struct HarbolByteBuf *buf);

HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_byte(struct HarbolByteBuf *buf, uint8_t byte);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_int16(struct HarbolByteBuf *buf, uint16_t integer);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_int32(struct HarbolByteBuf *buf, uint32_t integer);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_int64(struct HarbolByteBuf *buf, uint64_t integer);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_ptr(struct HarbolByteBuf *buf, uintptr_t intptr);

HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_float32(struct HarbolByteBuf *buf, float32_t f32);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_float64(struct HarbolByteBuf *buf, float64_t f64);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_floatmax(struct HarbolByteBuf *buf, floatmax_t fmax);

HARBOL_EXPORT NEVER_NULL(1) bool harbol_bytebuffer_insert_cstr(struct HarbolByteBuf *buf, const char cstr[]);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_obj(struct HarbolByteBuf *buf, const void *obj, size_t len);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_zeros(struct HarbolByteBuf *buf, size_t amount);

HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_del(struct HarbolByteBuf *buf, uindex_t index, size_t range);

HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_to_file(const struct HarbolByteBuf *buf, FILE *file);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_insert_from_file(struct HarbolByteBuf *buf, FILE *file);

HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_append(struct HarbolByteBuf *bufA, struct HarbolByteBuf *bufB);
HARBOL_EXPORT NO_NULL bool harbol_bytebuffer_copy(struct HarbolByteBuf *bufA, struct HarbolByteBuf *bufB);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_BYTEBUFFER_INCLUDED */
