#ifndef HARBOL_COMMON_INCLUDES_INCLUDED
#	define HARBOL_COMMON_INCLUDES_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>

// placing this here so we can get this after including inttypes.h
#if defined(INTPTR_MAX)
#	if defined(INT32_MAX) && INTPTR_MAX>=INT32_MAX
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
typedef struct { const char *CStr; const size_t Len; } string_t;
static inline NO_NULL string_t __string_create(const char cstr[static 1], const size_t len)
{
	return (string_t){ cstr, len };
}
#	ifndef string_create
#	define string_create(cstr_literal) \
			__string_create((cstr_literal), sizeof(cstr_literal) - 1)
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
#	endif
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
#	endif
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
#	endif
#endif


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
			uint8_t *const newdata = calloc(new_size, element_size);
			if( newdata==NULL ) {
				return false;
			} else {
				obj->len = new_size;
				// copy the old table to new then free old table.
				if( obj->tab != NULL ) {
					memcpy(newdata, obj->tab, element_size * old_size);
					free(obj->tab), obj->tab = NULL;
				}
				obj->tab = newdata;
				return true;
			}
		} else {
			uint8_t *result = realloc(obj->tab, element_size * new_size);
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

static inline NO_NULL size_t string_hash(const char key[static 1])
{
	const size_t hash_constant = 37; // 97, 33
	size_t h = 0;
	while( *key != '\0' )
		h = hash_constant * h + *key++;
	return h;
}

static inline uint32_t int32_hash(uint32_t a)
{
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);
	return a;
}

static inline uint64_t int64_hash(uint64_t a)
{
	a = (~a) + (a << 21);
	a = a ^ (a >> 24);
	a = (a + (a << 3)) + (a << 8);
	a = a ^ (a >> 14);
	a = (a + (a << 2)) + (a << 4);
	a = a ^ (a >> 28);
	a = a + (a << 31);
	return a;
}

static inline size_t int_hash(const size_t a)
{
	switch( sizeof a ) {
		case 4: return int32_hash(a);
		case 8: return int64_hash(a);
		default: return 0;
	}
}

static inline NO_NULL size_t ptr_hash(const void *const p)
{
	const size_t y = (size_t)p;
	return (y >> 4u) | (y << (8u * sizeof(void*) - 4u));
}

static inline NO_NULL size_t jenkins_one_at_a_time_hash(const char key[static 1], const size_t len)
{
	size_t hash = 0;
	for( uindex_t i=0; i<len; i++ ) {
		hash += key[i];
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}

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

static inline NO_NULL uint8_t *make_buffer_from_binary(const char file_name[restrict static 1])
{
	FILE *restrict file = fopen(file_name, "rb");
	if( file==NULL )
		return NULL;
	else {
		const ssize_t filesize = get_file_size(file);
		if( filesize <= -1 ) {
			fclose(file);
			return NULL;
		} else {
			uint8_t *restrict stream = calloc(filesize, sizeof *stream);
			const size_t bytes_read = fread(stream, sizeof *stream, filesize, file);
			fclose(file), file=NULL;
			
			if( bytes_read != (size_t)filesize ) {
				free(stream), stream=NULL;
				return NULL;
			}
			else return stream;
		}
	}
}

static inline bool is_decimal(const int c)
{
	return( c >= '0' && c <= '9' );
}

static inline bool is_octal(const int c)
{
	return( c >= '0' && c <= '7' );
}

static inline bool is_hex(const int c)
{
	return( (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || is_decimal(c) );
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
