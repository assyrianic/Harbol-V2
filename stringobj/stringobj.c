#include "stringobj.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

#include <ctype.h>

static NO_NULL bool __harbol_resize_string(struct HarbolString *const string, const size_t new_size)
{
	const size_t old_size = string->Len;
	// check if we're reducing or increasing memory.
	// as realloc is exponentially faster when we're reducing memory.
	const bool increasing_mem = (old_size <= new_size);
	if( increasing_mem ) {
		char *newstr = calloc(new_size + 1, sizeof *newstr);
		if( newstr==NULL )
			return false;
		else {
			string->Len = new_size;
			if( string->CStr != NULL ) {
				memcpy(newstr, string->CStr, old_size);
				free(string->CStr), string->CStr=NULL;
			}
			string->CStr = newstr;
			return true;
		}
	} else {
		string->CStr = realloc(string->CStr, new_size * sizeof *string->CStr + 1);
		if( string->CStr==NULL )
			return false;
		else {
			string->Len = new_size;
			string->CStr[string->Len] = '\0';
			return true;
		}
	}
}


HARBOL_EXPORT struct HarbolString *harbol_string_new(const char cstr[restrict])
{
	struct HarbolString *restrict string = calloc(1, sizeof *string);
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
	if( string->CStr != NULL )
		free(string->CStr), string->CStr=NULL;
	*string = (struct HarbolString)EMPTY_HARBOL_STRING;
	return true;
}

HARBOL_EXPORT bool harbol_string_free(struct HarbolString **const stringref)
{
	if( *stringref==NULL )
		return false;
	else {
		const bool res = harbol_string_clear(*stringref);
		free(*stringref), *stringref=NULL;
		return res && *stringref==NULL;
	}
}

HARBOL_EXPORT bool harbol_string_add_char(struct HarbolString *const string, const char c)
{
	const bool resize_res = __harbol_resize_string(string, string->Len + 1);
	if( !resize_res )
		return false;
	else {
		string->CStr[string->Len-1] = c;
		return true;
	}
}

HARBOL_EXPORT bool harbol_string_add_str(struct HarbolString *const stringA, const struct HarbolString *const stringB)
{
	if( stringB->CStr==NULL )
		return false;
	else {
		const bool resize_res = __harbol_resize_string(stringA, stringA->Len + stringB->Len);
		if( !resize_res )
			return false;
		else {
			strncat(stringA->CStr, stringB->CStr, stringB->Len);
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
		const bool resize_res = __harbol_resize_string(string, string->Len + cstr_len);
		if( !resize_res )
			return false;
		else {
			strncat(string->CStr, cstr, cstr_len);
			return true;
		}
	}
}

HARBOL_EXPORT inline char *harbol_string_cstr(const struct HarbolString *const string)
{
	return string->CStr;
}

HARBOL_EXPORT inline size_t harbol_string_len(const struct HarbolString *const string)
{
	return string->Len;
}

HARBOL_EXPORT bool harbol_string_copy_str(struct HarbolString *const stringA, const struct HarbolString *const stringB)
{
	if( stringB->CStr==NULL )
		return false;
	else if( stringA==stringB )
		return true;
	else {
		const bool resize_res = __harbol_resize_string(stringA, stringB->Len);
		if( !resize_res )
			return false;
		else {
			strncpy(stringA->CStr, stringB->CStr, stringB->Len);
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
			strncpy(string->CStr, cstr, string->Len);
			return true;
		}
	}
}

HARBOL_EXPORT int32_t harbol_string_format(struct HarbolString *const restrict string, const char fmt[restrict static 1], ...)
{
	va_list ap, st;
	va_start(ap, fmt);
	va_start(st, fmt);
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
		const int32_t result = vsnprintf(string->CStr, string->Len+1, fmt, st);
		va_end(st);
		return result;
	}
}

HARBOL_EXPORT int32_t harbol_string_add_format(struct HarbolString *const restrict string, const char fmt[restrict static 1], ...)
{
	va_list ap, st;
	va_start(ap, fmt);
	va_start(st, fmt);
	char c = 0;
	const int32_t size = vsnprintf(&c, 1, fmt, ap);
	va_end(ap);
	
	const size_t old_size = string->Len;
	const bool resize_res = __harbol_resize_string(string, size + old_size);
	if( !resize_res ) {
		va_end(st);
		return -1;
	} else {
		const int32_t result = vsnprintf(&string->CStr[old_size], string->Len-old_size+1, fmt, st);
		va_end(st);
		return result;
	}
}

HARBOL_EXPORT int32_t harbol_string_cmpcstr(const struct HarbolString *const restrict string, const char cstr[restrict])
{
	return( cstr==NULL || string->CStr==NULL ) ? -1 : strncmp(cstr, string->CStr, string->Len);
}

HARBOL_EXPORT int32_t harbol_string_cmpstr(const struct HarbolString *const restrict stringA, const struct HarbolString *const restrict stringB)
{
	return( stringA->CStr==NULL || stringB->CStr==NULL ) ? -1 : strncmp(stringA->CStr, stringB->CStr, stringB->Len);
}

HARBOL_EXPORT bool harbol_string_is_empty(const struct HarbolString *const string)
{
	return( string->CStr==NULL || string->Len==0 || string->CStr[0]==0 );
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
			string->Len = fread(string->CStr, sizeof *string->CStr, filesize, file);
			return true;
		}
	}
}

HARBOL_EXPORT bool harbol_string_replace(struct HarbolString *const string, const char to_replace, const char with)
{
	if( string->CStr==NULL || to_replace==0 || with==0 )
		return false;
	else {
		for( char *i=string->CStr; *i; i++ )
			if( *i==to_replace )
				*i = with;
		return true;
	}
}

HARBOL_EXPORT size_t harbol_string_count(struct HarbolString *const string, const char occurrence)
{
	if( string->CStr==NULL )
		return 0;
	else {
		size_t counts = 0;
		for( char *i=string->CStr; *i; i++ )
			if( *i==occurrence )
				++counts;
		return counts;
	}
}

HARBOL_EXPORT bool harbol_string_upper(struct HarbolString *const string)
{
	if( string->CStr==NULL )
		return false;
	else {
		for( char *i=string->CStr; *i; i++ )
			if( islower(*i) )
				*i=toupper(*i);
		return true;
	}
}

HARBOL_EXPORT bool harbol_string_lower(struct HarbolString *const string)
{
	if( string->CStr==NULL )
		return false;
	else {
		for( char *i=string->CStr; *i; i++ )
			if( isupper(*i) )
				*i=tolower(*i);
		return true;
	}
}

HARBOL_EXPORT bool harbol_string_reverse(struct HarbolString *const string)
{
	if( string->CStr==NULL )
		return false;
	else {
		char *buf = string->CStr;
		const size_t len = string->Len / 2;
		for( uindex_t i=0, n=string->Len-1; i<len; i++, n-- ) {
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
