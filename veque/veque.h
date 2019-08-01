#ifndef HARBOL_VEQUE_INCLUDED
#	define HARBOL_VEQUE_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"

// Vector + Queue = Veque!
struct HarbolVeque {
	uint8_t *Table;
	size_t Len, DataSize, Begin, End;
};

#define EMPTY_HARBOL_VEQUE    {NULL,0,0,0,0};

HARBOL_EXPORT struct HarbolVeque *harbol_veque_new(size_t datasize, size_t len);
HARBOL_EXPORT struct HarbolVeque harbol_veque_create(size_t datasize, size_t len);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_veque_clear(struct HarbolVeque *veque, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_veque_free(struct HarbolVeque **vequeref, void dtor(void**));

HARBOL_EXPORT NO_NULL bool harbol_veque_insert_front(struct HarbolVeque *veque, void *val);
HARBOL_EXPORT NO_NULL bool harbol_veque_insert_back(struct HarbolVeque *veque, void *val);

HARBOL_EXPORT NO_NULL void *harbol_veque_pop_front(struct HarbolVeque *veque);
HARBOL_EXPORT NO_NULL void *harbol_veque_pop_back(struct HarbolVeque *veque);

HARBOL_EXPORT NO_NULL void harbol_veque_reset(struct HarbolVeque *veque);

HARBOL_EXPORT NO_NULL size_t harbol_veque_count(const struct HarbolVeque *veque);
HARBOL_EXPORT NO_NULL bool harbol_veque_full(const struct HarbolVeque *veque);
HARBOL_EXPORT NO_NULL bool harbol_veque_empty(const struct HarbolVeque *veque);

HARBOL_EXPORT NO_NULL void *harbol_veque_get_front(const struct HarbolVeque *veque);
HARBOL_EXPORT NO_NULL void *harbol_veque_get_back(const struct HarbolVeque *veque);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_VEQUE_INCLUDED */
