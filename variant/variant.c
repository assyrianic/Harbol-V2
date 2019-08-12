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
	v.data = harbol_alloc(v.datasize, sizeof *v.data);
	if( v.data != NULL ) {
		memcpy(v.data, val, v.datasize);
	}
	return v;
}

HARBOL_EXPORT bool harbol_variant_clear(struct HarbolVariant *const variant, void dtor(void**))
{
	if( variant->datasize==0 || variant->data==NULL )
		return false;
	else {
		if( dtor != NULL )
			dtor((void**)&variant->data);
		if( variant->data != NULL )
			harbol_free(variant->data), variant->data=NULL;
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
	return( variant->datasize==0 || variant->data==NULL ) ? NULL : variant->data;
}

HARBOL_EXPORT bool harbol_variant_set(struct HarbolVariant *const restrict variant, void *const restrict val)
{
	if( variant->datasize==0 )
		return false;
	else if( variant->data==NULL ) {
		variant->data = harbol_alloc(variant->datasize, sizeof *variant->data);
		return( variant->data==NULL ) ? false : memcpy(variant->data, val, variant->datasize) != NULL;
	} else {
		return memcpy(variant->data, val, variant->datasize) != NULL;
	}
}

HARBOL_EXPORT NO_NULL int32_t harbol_variant_tag(const struct HarbolVariant *variant)
{
	return variant->tag;
}
