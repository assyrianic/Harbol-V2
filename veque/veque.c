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
	veque.Table = harbol_alloc(datasize, len);
	if( veque.Table==NULL )
		return veque;
	else {
		veque.Len = len;
		veque.DataSize = datasize;
		return veque;
	}
}

HARBOL_EXPORT bool harbol_veque_clear(struct HarbolVeque *const veque, void dtor(void**))
{
	if( dtor != NULL && veque->DataSize > 0 )
		for( uindex_t i=0; i<veque->Len; i++ )
			dtor((void**)&(uint8_t *){&veque->Table[i * veque->DataSize]});
	
	harbol_free(veque->Table), veque->Table = NULL;
	veque->Len = veque->Begin = veque->End = 0;
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
	if( veque->Table==NULL || veque->DataSize==0 || harbol_veque_full(veque) )
		return false;
	else {
		veque->Begin = (veque->Begin + veque->Len - 1) % veque->Len;
		return memcpy(&veque->Table[veque->Begin * veque->DataSize], val, veque->DataSize) != NULL;
	}
}

HARBOL_EXPORT bool harbol_veque_insert_back(struct HarbolVeque *const restrict veque, void *restrict val)
{
	if( veque->Table==NULL || veque->DataSize==0 || harbol_veque_full(veque) )
		return false;
	else {
		memcpy(&veque->Table[veque->End * veque->DataSize], val, veque->DataSize);
		veque->End = (veque->End + 1) % veque->Len;
		return true;
	}
}

HARBOL_EXPORT void *harbol_veque_pop_front(struct HarbolVeque *const veque)
{
	if( veque->Table==NULL || veque->DataSize==0 || harbol_veque_empty(veque) )
		return NULL;
	else {
		uint8_t *const p = &veque->Table[veque->Begin * veque->DataSize];
		veque->Begin = (veque->Begin + 1) % veque->Len;
		return p;
	}
}

HARBOL_EXPORT void *harbol_veque_pop_back(struct HarbolVeque *const veque)
{
	if( veque->Table==NULL || veque->DataSize==0 || harbol_veque_empty(veque) )
		return NULL;
	else {
		veque->End = (veque->End + veque->Len - 1) % veque->Len;
		return &veque->Table[veque->End * veque->DataSize];
	}
}

HARBOL_EXPORT void harbol_veque_reset(struct HarbolVeque *const veque)
{
	veque->End = veque->Begin = 0;
}

HARBOL_EXPORT size_t harbol_veque_count(const struct HarbolVeque *const veque)
{
	return (veque->End + veque->Len - veque->Begin) % veque->Len;
}

HARBOL_EXPORT bool harbol_veque_full(const struct HarbolVeque *const veque)
{
	return harbol_veque_count(veque)==(veque->Len - 1);
}

HARBOL_EXPORT bool harbol_veque_empty(const struct HarbolVeque *const veque)
{
	return harbol_veque_count(veque)==0;
}

HARBOL_EXPORT void *harbol_veque_get_front(const struct HarbolVeque *const veque)
{
	return( veque->Table != NULL && veque->DataSize > 0 ) ? &veque->Table[veque->Begin * veque->DataSize] : NULL;
}

HARBOL_EXPORT void *harbol_veque_get_back(const struct HarbolVeque *const veque)
{
	return( veque->Table != NULL && veque->DataSize > 0 ) ? &veque->Table[veque->End * veque->DataSize] : NULL;
}
