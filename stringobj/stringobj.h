#ifndef HARBOL_STRING_INCLUDED
#	define HARBOL_STRING_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"

struct HarbolString {
	char *CStr;
	size_t Len;
};

#define EMPTY_HARBOL_STRING    { NULL,0 }


HARBOL_EXPORT struct HarbolString *harbol_string_new(const char cstr[]);
HARBOL_EXPORT struct HarbolString harbol_string_create(const char cstr[]);

HARBOL_EXPORT NO_NULL bool harbol_string_clear(struct HarbolString *str);
HARBOL_EXPORT NO_NULL bool harbol_string_free(struct HarbolString **strref);

HARBOL_EXPORT NO_NULL char *harbol_string_cstr(const struct HarbolString *str);
HARBOL_EXPORT NO_NULL size_t harbol_string_len(const struct HarbolString *str);

HARBOL_EXPORT NO_NULL bool harbol_string_add_char(struct HarbolString *str, char chr);
HARBOL_EXPORT NO_NULL bool harbol_string_add_str(struct HarbolString *strA, const struct HarbolString *strB) ;
HARBOL_EXPORT NEVER_NULL(1) bool harbol_string_add_cstr(struct HarbolString *str, const char cstr[]);

HARBOL_EXPORT NO_NULL bool harbol_string_copy_str(struct HarbolString *strA, const struct HarbolString *strB);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_string_copy_cstr(struct HarbolString *str, const char cstr[]);

HARBOL_EXPORT NEVER_NULL(1, 2) int32_t harbol_string_format(struct HarbolString *str, const char fmt[], ...);
HARBOL_EXPORT NEVER_NULL(1, 2) int32_t harbol_string_add_format(struct HarbolString *str, const char fmt[], ...);
HARBOL_EXPORT NEVER_NULL(1) int32_t harbol_string_cmpcstr(const struct HarbolString *str, const char cstr[]);
HARBOL_EXPORT NO_NULL int32_t harbol_string_cmpstr(const struct HarbolString *strA, const struct HarbolString *strB);

HARBOL_EXPORT NO_NULL bool harbol_string_is_empty(const struct HarbolString *str);

HARBOL_EXPORT NO_NULL bool harbol_string_read_file(struct HarbolString *str, FILE *file);
HARBOL_EXPORT NO_NULL bool harbol_string_replace(struct HarbolString *str, char to_replace, char with);
HARBOL_EXPORT NO_NULL size_t harbol_string_count(struct HarbolString *str, char occurrence);

HARBOL_EXPORT NO_NULL bool harbol_string_upper(struct HarbolString *str);
HARBOL_EXPORT NO_NULL bool harbol_string_lower(struct HarbolString *str);
HARBOL_EXPORT NO_NULL bool harbol_string_reverse(struct HarbolString *str);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_STRING_INCLUDED */
