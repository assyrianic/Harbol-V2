#include "veque.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolVeque *harbol_veque_new(const size_t datasize, const size_t len)
{
	struct HarbolVeque *veque = harbol_alloc(1, sizeof *veque);
	if( veque != NULL )
		*veque = harbol_veque_create(datasize, len);
	return veque;
}

HARBOL_EXPORT struct HarbolVeque harbol_veque_create(const size_t datasize, const size_t len)
{
	struct HarbolVeque veque = EMPTY_HARBOL_VEQUE;
	veque.table = harbol_alloc(datasize, len);
	if( veque.table==NULL )
		return veque;
	else {
		veque.len = len;
		veque.datasize = datasize;
		return veque;
	}
}

HARBOL_EXPORT bool harbol_veque_clear(struct HarbolVeque *const veque, void dtor(void**))
{
	if( dtor != NULL && veque->datasize > 0 )
		for( uindex_t i=0; i<veque->len; i++ )
			dtor((void**)&(uint8_t *){&veque->table[i * veque->datasize]});
	
	harbol_free(veque->table), veque->table = NULL;
	veque->len = veque->begin = veque->end = 0;
	return true;
}

HARBOL_EXPORT bool harbol_veque_free(struct HarbolVeque **const vequeref, void dtor(void**))
{
	if( *vequeref==NULL )
		return false;
	else {
		const bool res = harbol_veque_clear(*vequeref, dtor);
		harbol_free(*vequeref), *vequeref=NULL;
		return res;
	}
}

HARBOL_EXPORT bool harbol_veque_insert_front(struct HarbolVeque *const restrict veque, void *restrict val)
{
	if( veque->table==NULL || veque->datasize==0 || harbol_veque_full(veque) )
		return false;
	else {
		veque->begin = (veque->begin + veque->len - 1) % veque->len;
		return memcpy(&veque->table[veque->begin * veque->datasize], val, veque->datasize) != NULL;
	}
}

HARBOL_EXPORT bool harbol_veque_insert_back(struct HarbolVeque *const restrict veque, void *restrict val)
{
	if( veque->table==NULL || veque->datasize==0 || harbol_veque_full(veque) )
		return false;
	else {
		memcpy(&veque->table[veque->end * veque->datasize], val, veque->datasize);
		veque->end = (veque->end + 1) % veque->len;
		return true;
	}
}

HARBOL_EXPORT void *harbol_veque_pop_front(struct HarbolVeque *const veque)
{
	if( veque->table==NULL || veque->datasize==0 || harbol_veque_empty(veque) )
		return NULL;
	else {
		uint8_t *const p = &veque->table[veque->begin * veque->datasize];
		veque->begin = (veque->begin + 1) % veque->len;
		return p;
	}
}

HARBOL_EXPORT void *harbol_veque_pop_back(struct HarbolVeque *const veque)
{
	if( veque->table==NULL || veque->datasize==0 || harbol_veque_empty(veque) )
		return NULL;
	else {
		veque->end = (veque->end + veque->len - 1) % veque->len;
		return &veque->table[veque->end * veque->datasize];
	}
}

HARBOL_EXPORT void harbol_veque_reset(struct HarbolVeque *const veque)
{
	veque->end = veque->begin = 0;
}

HARBOL_EXPORT size_t harbol_veque_count(const struct HarbolVeque *const veque)
{
	return (veque->end + veque->len - veque->begin) % veque->len;
}

HARBOL_EXPORT bool harbol_veque_full(const struct HarbolVeque *const veque)
{
	return harbol_veque_count(veque)==(veque->len - 1);
}

HARBOL_EXPORT bool harbol_veque_empty(const struct HarbolVeque *const veque)
{
	return harbol_veque_count(veque)==0;
}

HARBOL_EXPORT void *harbol_veque_get_front(const struct HarbolVeque *const veque)
{
	return( veque->table != NULL && veque->datasize > 0 ) ? &veque->table[veque->begin * veque->datasize] : NULL;
}

HARBOL_EXPORT void *harbol_veque_get_back(const struct HarbolVeque *const veque)
{
	return( veque->table != NULL && veque->datasize > 0 ) ? &veque->table[veque->end * veque->datasize] : NULL;
}
