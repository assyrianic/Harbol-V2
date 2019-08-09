#ifndef HARBOL_COMMON_INCLUDES_INCLUDED
#	define HARBOL_COMMON_INCLUDES_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>

/* placing this here so we can get this after including inttypes.h */
#if defined(INTPTR_MAX)
#	if defined(INT32_MAX) && INTPTR_MAX==INT32_MAX
#		ifndef HARBOL32
#			define HARBOL32
#		endif
#	endif
#	if defined(INT64_MAX) && INTPTR_MAX==INT64_MAX
#		ifndef HARBOL64
#			define HARBOL64
#		endif
#	endif
#endif

/* types as defined by Harbol. */
#ifndef __index_t_defined
#	define __index_t_defined
typedef intptr_t index_t;
#endif

#ifndef __uindex_t_defined
#	define __uindex_t_defined
typedef uintptr_t uindex_t;
#endif

#ifndef __ssize_t_defined
typedef intptr_t ssize_t;
#	define __ssize_t_defined
#endif

#ifndef __string_t_defined
#	define __string_t_defined
typedef struct { const char *cstr; const size_t len; } string_t;
#	ifndef string_create
#		define string_create(cstr_literal)    (string_t){(cstr_literal), sizeof(cstr_literal) - 1}
#	endif
#endif


/* According to C99 standards.
 * there are three floating point types: float, double, and long double.
 * 
 * The type double provides at least as much precision as float, and the type long double provides at least as much precision as double.
 * 
 * so in summary: float <= double <= long double
 */
#ifndef __float32_t_defined
#	if FLT_MANT_DIG==24
#		define __float32_t_defined
#		define PRIf32 "f"
#		define strtof32  strtof
		typedef float float32_t;
#	elif DBL_MANT_DIG==24
#		define __float32_t_defined
#		define PRIf32 "f"
#		define strtof32  strtod
		typedef double float32_t;
#	else
#		error "no appropriate float32 implementation"
#	endif
#endif

#ifdef C11
	_Static_assert(sizeof(float32_t) * CHAR_BIT == 32, "Unexpected `float32_t` size");
#endif


#ifndef __float64_t_defined
#	if DBL_MANT_DIG==53
#		define __float64_t_defined
#		define PRIf64    "f"
#		define strtof64  strtod
		typedef double float64_t;
#	elif LDBL_MANT_DIG==53
#		define __float64_t_defined
#		define PRIf64    "Lf"
#		define strtof64  strtold
		typedef long double float64_t;
// This is unlikely but check just in case.
#	elif FLT_MANT_DIG==53
#		define __float64_t_defined
#		define PRIf64    "f"
#		define strtof64  strtof
		typedef float float64_t;
#	else
#		error "no appropriate float64 implementation"
#	endif
#endif

#ifdef C11
	_Static_assert(sizeof(float64_t) * CHAR_BIT == 64, "Unexpected `float64_t` size");
#endif


#ifndef __floatptr_t_defined
#	define PRIfPTR    "f"
#	if defined(HARBOL64)
#		define __floatptr_t_defined
#		define strtofptr  strtod
		typedef float64_t floatptr_t;
#	elif defined(HARBOL32)
#		define __floatptr_t_defined
#		define strtofptr  strtof
		typedef float32_t floatptr_t;
#	else
#		error "no appropriate floatptr implementation"
#	endif
#endif

#ifdef C11
	_Static_assert(sizeof(floatptr_t) * CHAR_BIT == sizeof(intptr_t), "Unexpected `floatptr_t` size");
#endif


#ifndef __floatmax_t_defined
#	if LDBL_MANT_DIG > DBL_MANT_DIG
#		define __floatmax_t_defined
#		define PRIfMAX    "Lf"
#		define strtofmax  strtold
		typedef long double floatmax_t;
#	elif DBL_MANT_DIG==LDBL_MANT_DIG && DBL_MANT_DIG > FLT_MANT_DIG
#		define __floatmax_t_defined
#		define PRIfMAX    "f"
#		define strtofmax  strtod
		typedef double floatmax_t;
#	elif DBL_MANT_DIG==FLT_MANT_DIG
#		define __floatmax_t_defined
#		define PRIfMAX    "f"
#		define strtofmax  strtof
		typedef float floatmax_t;
#	else
#		error "no appropriate floatmax implementation"
#	endif
#endif


//#define HARBOL_USE_MEMPOOL

#ifdef HARBOL_USE_MEMPOOL
	struct HarbolMemPool;
	extern struct HarbolMemPool *g_pool;
#endif

inline void *harbol_alloc(const size_t num, const size_t size)
{
#ifdef HARBOL_USE_MEMPOOL
	void *harbol_mempool_alloc(struct HarbolMemPool *mempool, size_t bytes);
	return harbol_mempool_alloc(g_pool, num * size);
#else
	return calloc(num, size);
#endif
}

inline void *harbol_realloc(void *const ptr, const size_t bytes)
{
#ifdef HARBOL_USE_MEMPOOL
	void *harbol_mempool_realloc(struct HarbolMemPool *mempool, void *ptr, size_t bytes);
	return harbol_mempool_realloc(g_pool, ptr, bytes);
#else
	return realloc(ptr, bytes);
#endif
}

inline void harbol_free(void *const ptr)
{
#ifdef HARBOL_USE_MEMPOOL
	bool harbol_mempool_free(struct HarbolMemPool *mempool, void *ptr);
	harbol_mempool_free(g_pool, ptr);
#else
	free(ptr);
#endif
}

inline void harbol_clean(void **const ptrref)
{
#ifdef HARBOL_USE_MEMPOOL
	bool harbol_mempool_cleanup(struct HarbolMemPool *mempool, void **ptrref);
	harbol_mempool_cleanup(g_pool, ptrref);
#else
	free(*ptrref);
#endif
	*ptrref = NULL;
}

static inline bool harbol_generic_vector_resizer(void *const vec, const size_t new_size, const size_t element_size)
{
	struct {
		uint8_t *tab;
		size_t len;
	} *restrict obj = vec;
	
	// first we get our old size.
	// then resize the actual size.
	const size_t old_size = obj->len;
	if( old_size==new_size )
		return true;
	else {
		const bool increasing_mem = (old_size < new_size);
		if( increasing_mem ) {
			// allocate new table.
			uint8_t *const newdata = harbol_alloc(new_size, element_size);
			if( newdata==NULL ) {
				return false;
			} else {
				obj->len = new_size;
				// copy the old table to new then free old table.
				if( obj->tab != NULL ) {
					memcpy(newdata, obj->tab, element_size * old_size);
					harbol_free(obj->tab), obj->tab = NULL;
				}
				obj->tab = newdata;
				return true;
			}
		} else {
			uint8_t *result = harbol_realloc(obj->tab, element_size * new_size);
			if( result==NULL ) {
				return false;
			} else {
				if( result != obj->tab )
					obj->tab = result;
				obj->len = new_size;
				return true;
			}
		}
	}
}

static inline size_t harbol_align_size(const size_t size, const size_t align)
{
	return (size + (align-1)) & -align;
}


// these are NOT cryptographic hashes.
// use ONLY FOR HASH TABLE IMPLEMENTATIONS.
static inline NO_NULL size_t string_hash(const char key[static 1])
{
	// some hashing constants.
	// 37, 97, 33
	// using 37 because it brings better distribution with bitshift at return.
	const size_t hash_constant = 37;
	size_t h = 0;
	while( *key != '\0' )
		h = hash_constant * h + *key++;
	return h >> 1;
}

static inline size_t int_hash(size_t a)
{
	return (((a ^ (a>>4)) ^ 0xdeadbeef) + ((a ^ (a>>4))<<5)) ^ ((a>>11) * 37);
}

static inline size_t float_hash(const floatptr_t a)
{
	union {
		const floatptr_t f;
		const size_t s;
	} c = {a};
	return int_hash(c.s);
}

static inline NO_NULL size_t ptr_hash(const void *const p)
{
	union {
		const void *const p;
		const size_t y;
	} c = {p};
	return (c.y >> 4u) | (c.y << (8u * sizeof(void*) - 4u));
}

#ifdef C11
#	define harbol_hash(h)   _Generic((h)+0, \
								int : int_hash, \
								size_t : int_hash, \
								int64_t : int_hash, \
								uint64_t : int_hash, \
								float32_t : float_hash, \
								float64_t : float_hash, \
								floatptr_t : float_hash, \
								char* : string_hash, \
								const char* : string_hash, \
								default: ptr_hash) \
							((h))
#endif


static inline NO_NULL ssize_t get_file_size(FILE *const file)
{
	fseek(file, 0, SEEK_END);
	const ssize_t filesize = ftell(file);
	if( filesize<=0 )
		return -1;
	else {
		rewind(file);
		return filesize;
	}
}

// Binary Iterator Union.
// for "struct/union" types, cast from the 'Void' alias.
union HarbolBinIter {
	bool *restrict Bool;
	
	uint8_t *restrict UInt8; int8_t *restrict Int8;
	uint16_t *restrict UInt16; int16_t *restrict Int16;
	uint32_t *restrict UInt32; int32_t *restrict Int32;
	uint64_t *restrict UInt64; int64_t *restrict Int64;
	size_t *restrict Size; ssize_t *restrict SSize;
	uintptr_t *restrict UIntPtr; intptr_t *restrict IntPtr;
	
	float32_t *restrict Float32;
	float64_t *restrict Float64;
	floatptr_t *restrict FloatPtr;
	floatmax_t *restrict FloatMax;
	
	char *restrict String;
	
	void *restrict Void;
	union HarbolBinIter *restrict Self;
};

static inline NO_NULL uint8_t *make_buffer_from_binary(const char file_name[restrict static 1])
{
	FILE *restrict file = fopen(file_name, "rb");
	if( file==NULL )
		return NULL;
	else {
		const ssize_t filesize = get_file_size(file);
		if( filesize<=0 ) {
			fclose(file);
			return NULL;
		} else {
			uint8_t *restrict stream = harbol_alloc(filesize, sizeof *stream);
			const size_t bytes_read = fread(stream, sizeof *stream, filesize, file);
			fclose(file), file=NULL;
			
			if( bytes_read != (size_t)filesize ) {
				harbol_free(stream), stream=NULL;
				return NULL;
			}
			else return stream;
		}
	}
}

static inline NO_NULL char *make_buffer_from_text(const char file_name[restrict static 1])
{
	FILE *restrict file = fopen(file_name, "r");
	if( file==NULL )
		return NULL;
	else {
		const ssize_t filesize = get_file_size(file);
		if( filesize<=0 ) {
			fclose(file);
			return NULL;
		} else {
			char *restrict stream = harbol_alloc(filesize, sizeof *stream);
			const size_t bytes_read = fread(stream, sizeof *stream, filesize, file);
			fclose(file), file=NULL;
			
			if( bytes_read != (size_t)filesize ) {
				harbol_free(stream), stream=NULL;
				return NULL;
			}
			else return stream;
		}
	}
}

static inline bool is_decimal(const int c)
{
	return( c>='0' && c<='9' );
}

static inline bool is_octal(const int c)
{
	return( c>='0' && c<='7' );
}

static inline bool is_hex(const int c)
{
	return( (c>='a' && c<='f') || (c>='A' && c<='F') || is_decimal(c) );
}

static inline bool is_whitespace(const int c)
{
	return( c==' ' || c=='\t' || c=='\r' || c=='\v' || c=='\f' || c=='\n' );
}

static inline bool is_aligned(const void *const ptr, const size_t bytes)
{
	return ((uintptr_t)ptr & (bytes-1))==0;
}

#endif /* HARBOL_COMMON_INCLUDES_INCLUDED */
