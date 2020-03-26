#include "stringobj.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

#include <ctype.h>

static NO_NULL bool __harbol_resize_string(struct HarbolString *const string, const size_t new_size)
{
	const size_t old_size = string->len;
	// check if we're reducing or increasing memory.
	// as realloc is exponentially faster when we're reducing memory.
	const bool increasing_mem = (old_size <= new_size);
	if( increasing_mem ) {
		char *newstr = harbol_alloc(new_size + 1, sizeof *newstr);
		if( newstr==NULL )
			return false;
		else {
			string->len = new_size;
			if( string->cstr != NULL ) {
				memcpy(newstr, string->cstr, old_size);
				harbol_free(string->cstr), string->cstr=NULL;
			}
			string->cstr = newstr;
			return true;
		}
	} else {
		string->cstr = harbol_realloc(string->cstr, new_size * sizeof *string->cstr + 1);
		if( string->cstr==NULL )
			return false;
		else {
			string->len = new_size;
			string->cstr[string->len] = '\0';
			return true;
		}
	}
}


HARBOL_EXPORT struct HarbolString *harbol_string_new(const char cstr[restrict])
{
	struct HarbolString *restrict string = harbol_alloc(1, sizeof *string);
	if( string != NULL )
		*string = harbol_string_create(cstr);
	return string;
}

HARBOL_EXPORT struct HarbolString harbol_string_create(const char cstr[restrict])
{
	struct HarbolString string = EMPTY_HARBOL_STRING;
	harbol_string_copy_cstr(&string, cstr);
	return string;
}

HARBOL_EXPORT bool harbol_string_clear(struct HarbolString *const string)
{
	if( string->cstr != NULL )
		harbol_free(string->cstr), string->cstr=NULL;
	*string = (struct HarbolString)EMPTY_HARBOL_STRING;
	return true;
}

HARBOL_EXPORT bool harbol_string_free(struct HarbolString **const stringref)
{
	if( *stringref==NULL )
		return false;
	else {
		const bool res = harbol_string_clear(*stringref);
		harbol_free(*stringref), *stringref=NULL;
		return res && *stringref==NULL;
	}
}

HARBOL_EXPORT bool harbol_string_add_char(struct HarbolString *const string, const char c)
{
	const bool resize_res = __harbol_resize_string(string, string->len + 1);
	if( !resize_res )
		return false;
	else {
		string->cstr[string->len-1] = c;
		return true;
	}
}

HARBOL_EXPORT bool harbol_string_add_str(struct HarbolString *const stringA, const struct HarbolString *const stringB)
{
	if( stringB->cstr==NULL )
		return false;
	else {
		const bool resize_res = __harbol_resize_string(stringA, stringA->len + stringB->len);
		if( !resize_res )
			return false;
		else {
			strncat(stringA->cstr, stringB->cstr, stringB->len);
			return true;
		}
	}
}

HARBOL_EXPORT bool harbol_string_add_cstr(struct HarbolString *const restrict string, const char cstr[restrict])
{
	if( cstr==NULL )
		return false;
	else {
		const size_t cstr_len = strlen(cstr);
		const bool resize_res = __harbol_resize_string(string, string->len + cstr_len);
		if( !resize_res )
			return false;
		else {
			strncat(string->cstr, cstr, cstr_len);
			return true;
		}
	}
}

HARBOL_EXPORT inline char *harbol_string_cstr(const struct HarbolString *const string)
{
	return string->cstr;
}

HARBOL_EXPORT inline size_t harbol_string_len(const struct HarbolString *const string)
{
	return string->len;
}

HARBOL_EXPORT bool harbol_string_copy_str(struct HarbolString *const stringA, const struct HarbolString *const stringB)
{
	if( stringB->cstr==NULL )
		return false;
	else if( stringA==stringB )
		return true;
	else {
		const bool resize_res = __harbol_resize_string(stringA, stringB->len);
		if( !resize_res )
			return false;
		else {
			strncpy(stringA->cstr, stringB->cstr, stringB->len);
			return true;
		}
	}
}

HARBOL_EXPORT bool harbol_string_copy_cstr(struct HarbolString *const restrict string, const char cstr[restrict])
{
	if( cstr==NULL )
		return false;
	else {
		const size_t cstr_len = strlen(cstr);
		const bool resize_res = __harbol_resize_string(string, cstr_len);
		if( !resize_res )
			return false;
		else {
			strncpy(string->cstr, cstr, string->len);
			return true;
		}
	}
}

HARBOL_EXPORT int32_t harbol_string_format(struct HarbolString *const restrict string, const char fmt[restrict static 1], ...)
{
	va_list ap, st;
	va_start(ap, fmt);
	va_copy(st, ap);
	/*
		'*snprintf' family returns the size of how large the writing
		would be if the buffer was large enough.
	*/
	char c = 0;
	const int32_t size = vsnprintf(&c, 1, fmt, ap);
	va_end(ap);
	
	const bool resize_res = __harbol_resize_string(string, size);
	if( !resize_res ) {
		va_end(st);
		return -1;
	} else {
		/* vsnprintf always checks n-1 so gotta increase len a bit to accomodate. */
		const int32_t result = vsnprintf(string->cstr, string->len+1, fmt, st);
		va_end(st);
		return result;
	}
}

HARBOL_EXPORT int32_t harbol_string_add_format(struct HarbolString *const restrict str, const char fmt[restrict static 1], ...)
{
	va_list ap, st;
	va_start(ap, fmt);
	va_copy(st, ap);
	
	char c = 0;
	const int32_t size = vsnprintf(&c, 1, fmt, ap);
	va_end(ap);
	
	const size_t old_size = str->len;
	const bool resize_res = __harbol_resize_string(str, size + old_size);
	if( !resize_res ) {
		va_end(st);
		return -1;
	} else {
		const int32_t result = vsnprintf(&str->cstr[old_size], str->len-old_size+1, fmt, st);
		va_end(st);
		return result;
	}
}

HARBOL_EXPORT int32_t harbol_string_scan(struct HarbolString *const restrict string, const char fmt[restrict static 1], ...)
{
	va_list args;
	va_start(args, fmt);
	const int32_t result = vsscanf(string->cstr, fmt, args);
	va_end(args);
	return result;
}

HARBOL_EXPORT int32_t harbol_string_cmpcstr(const struct HarbolString *const restrict string, const char cstr[restrict])
{
	if( cstr==NULL || string->cstr==NULL )
		return -1;
	else {
		const size_t cstr_len = strlen(cstr);
		return strncmp(cstr, string->cstr, (string->len > cstr_len) ? string->len : cstr_len);
	}
}

HARBOL_EXPORT int32_t harbol_string_cmpstr(const struct HarbolString *const restrict stringA, const struct HarbolString *const restrict stringB)
{
	return( stringA->cstr==NULL || stringB->cstr==NULL ) ? -1 : strncmp(stringA->cstr, stringB->cstr, stringA->len > stringB->len ? stringA->len : stringB->len);
}

HARBOL_EXPORT bool harbol_string_is_empty(const struct HarbolString *const string)
{
	return( string->cstr==NULL || string->len==0 || string->cstr[0]==0 );
}

HARBOL_EXPORT bool harbol_string_read_file(struct HarbolString *const string, FILE *const file)
{
	const ssize_t filesize = get_file_size(file);
	if( filesize<=0 )
		return false;
	else {
		const bool resize_res = __harbol_resize_string(string, filesize);
		if( !resize_res )
			return false;
		else {
			string->len = fread(string->cstr, sizeof *string->cstr, filesize, file);
			return true;
		}
	}
}

HARBOL_EXPORT bool harbol_string_replace(struct HarbolString *const string, const char to_replace, const char with)
{
	if( string->cstr==NULL || to_replace==0 || with==0 )
		return false;
	else {
		for( char *i=string->cstr; *i; i++ )
			if( *i==to_replace )
				*i = with;
		return true;
	}
}

HARBOL_EXPORT size_t harbol_string_count(const struct HarbolString *const string, const char occurrence)
{
	if( string->cstr==NULL )
		return 0;
	else {
		size_t counts = 0;
		for( char *i=string->cstr; *i; i++ )
			if( *i==occurrence )
				++counts;
		return counts;
	}
}

HARBOL_EXPORT bool harbol_string_upper(struct HarbolString *const string)
{
	if( string->cstr==NULL )
		return false;
	else {
		for( char *i=string->cstr; *i; i++ )
			if( islower(*i) )
				*i=toupper(*i);
		return true;
	}
}

HARBOL_EXPORT bool harbol_string_lower(struct HarbolString *const string)
{
	if( string->cstr==NULL )
		return false;
	else {
		for( char *i=string->cstr; *i; i++ )
			if( isupper(*i) )
				*i=tolower(*i);
		return true;
	}
}

HARBOL_EXPORT bool harbol_string_reverse(struct HarbolString *const string)
{
	if( string->cstr==NULL )
		return false;
	else {
		char *buf = string->cstr;
		const size_t len = string->len / 2;
		for( uindex_t i=0, n=string->len-1; i<len; i++, n-- ) {
			if( buf[n]==buf[i] )
				continue;
			else {
				buf[n] ^= buf[i];
				buf[i] ^= buf[n];
				buf[n] ^= buf[i];
			}
		}
		return true;
	}
}
