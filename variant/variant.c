#include "variant.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolVariant *harbol_variant_new(void *const restrict val, const size_t datasize, const int32_t type_flags)
{
	struct HarbolVariant *v = harbol_alloc(1, sizeof *v);
	if( v != NULL )
		*v = harbol_variant_create(val, datasize, type_flags);
	return v;
}

HARBOL_EXPORT struct HarbolVariant harbol_variant_create(void *const restrict val, const size_t datasize, const int32_t type_flags)
{
	struct HarbolVariant v = {NULL, datasize, type_flags};
	v.Data = harbol_alloc(v.Size, sizeof *v.Data);
	if( v.Data != NULL ) {
		memcpy(v.Data, val, v.Size);
	}
	return v;
}

HARBOL_EXPORT bool harbol_variant_clear(struct HarbolVariant *const variant, void dtor(void**))
{
	if( variant->Size==0 || variant->Data==NULL )
		return false;
	else {
		if( dtor != NULL )
			dtor((void**)&variant->Data);
		if( variant->Data != NULL )
			harbol_free(variant->Data), variant->Data=NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_variant_free(struct HarbolVariant **const variantref, void dtor(void**))
{
	harbol_variant_clear(*variantref, dtor);
	harbol_free(*variantref), *variantref=NULL;
	return true;
}

HARBOL_EXPORT void *harbol_variant_get(const struct HarbolVariant *const variant)
{
	return( variant->Size==0 || variant->Data==NULL ) ? NULL : variant->Data;
}

HARBOL_EXPORT bool harbol_variant_set(struct HarbolVariant *const restrict variant, void *const restrict val)
{
	if( variant->Size==0 )
		return false;
	else if( variant->Data==NULL ) {
		variant->Data = harbol_alloc(variant->Size, sizeof *variant->Data);
		return( variant->Data==NULL ) ? false : memcpy(variant->Data, val, variant->Size) != NULL;
	} else {
		return memcpy(variant->Data, val, variant->Size) != NULL;
	}
}

HARBOL_EXPORT NO_NULL int32_t harbol_variant_tag(const struct HarbolVariant *variant)
{
	return variant->Type;
}
