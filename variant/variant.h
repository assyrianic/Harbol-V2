#ifndef HARBOL_VARIANT_INCLUDED
#	define HARBOL_VARIANT_INCLUDED

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"

#ifdef __cplusplus
extern "C" {
#endif

enum HarbolType {
	HarbolType_Nil = 0,
	
	HarbolType_Bool, HarbolType_Int8, HarbolType_UInt8,
	
	HarbolType_Int16, HarbolType_UInt16,
	
	HarbolType_Int32, HarbolType_UInt32, HarbolType_Float32,
	
	HarbolType_Int64, HarbolType_UInt64, HarbolType_Float64,
	
	HarbolType_Float128,
	
	HarbolType_IntPtr, HarbolType_UIntPtr, HarbolType_Size,
	
	HarbolType_Func, HarbolType_Struct, HarbolType_Union,
	
	// continue your own enum type with this.
	HarbolType_UserDefd,
	
	// use these ones only as flags.
	HarbolType_Array = 0x20000000,
	HarbolType_Ptr = 0x40000000,
};

struct HarbolVariant {
	uint8_t *Data; size_t Size;
	int32_t Type;
};


HARBOL_EXPORT NO_NULL struct HarbolVariant *harbol_variant_new(void *val, size_t datasize, int32_t type_flags);
HARBOL_EXPORT NO_NULL struct HarbolVariant harbol_variant_create(void *val, size_t datasize, int32_t type_flags);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_variant_clear(struct HarbolVariant *variant, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_variant_free(struct HarbolVariant **variantref, void dtor(void**));

HARBOL_EXPORT NO_NULL void *harbol_variant_get(const struct HarbolVariant *variant);
HARBOL_EXPORT NO_NULL bool harbol_variant_set(struct HarbolVariant *variant, void *val);
HARBOL_EXPORT NO_NULL int32_t harbol_variant_tag(const struct HarbolVariant *variant);


/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_VARIANT_INCLUDED */
