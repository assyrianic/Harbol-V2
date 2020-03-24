#include "cfg.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

/* CFG Parser in EBNF grammar
	keyval = <string> [':'] (<value>|<section>) [','] ;
	section = '{' *<keyval> '}' ;
	value = <string> | <number> | <vec> | "true" | "false" | "null" | "iota" ;
	matrix = '[' <number> [','] [<number>] [','] [<number>] [','] [<number>] ']' ;
	vec = ('v' | 'c') <matrix> ;
	string = '"' chars '"' | "'" chars "'" ;
*/

#ifndef HARBOL_CFG_ERR_STK_SIZE
#	define HARBOL_CFG_ERR_STK_SIZE 20
#endif

static struct {
	struct HarbolString errs[HARBOL_CFG_ERR_STK_SIZE];
	size_t count, curr_line;
} _g_cfg_err;

static struct {
	intmax_t global, *local;
} _g_iota;


static NO_NULL bool skip_ws_and_comments(const char **strref)
{
	if( *strref==NULL || **strref==0 ) {
		return false;
	} else {
		while( **strref != 0 && (is_whitespace(**strref) || // white space
				**strref=='#' || (**strref=='/' && (*strref)[1]=='/') || // single line comment
				(**strref=='/' && (*strref)[1]=='*') || // multi-line comment
				**strref==':' || **strref==',') ) // delimiters.
		{
			if( is_whitespace(**strref) ) {
				if( **strref=='\n' )
					_g_cfg_err.curr_line++;
				*strref = skip_chars(*strref, is_whitespace);
			} else if( **strref=='#' || (**strref=='/' && (*strref)[1]=='/') ) {
				*strref = skip_single_line_comment(*strref);
				_g_cfg_err.curr_line++;
			} else if( **strref=='/' && (*strref)[1]=='*' )
				*strref = skip_multi_line_comment(*strref, "*/", sizeof "*/"-1);
			else if( **strref==':' || **strref==',' )
				(*strref)++;
		}
		return **strref != 0;
	}
}

/*
static NEVER_NULL(1) bool _lex_string(const char **restrict strref, struct HarbolString *const restrict str)
{
	if( *strref==NULL || **strref==0 || str==NULL )
		return false;
	else if( !(**strref == '"' || **strref == '\'') ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid string quote mark: '%c'. Line: %zu\n", **strref, _g_cfg_err.curr_line);
		return false;
	}
	const char quote = *(*strref)++;
	while( **strref != 0 && **strref != quote ) {
		const char chrval = *(*strref)++;
		if( chrval=='\\' ) {
			const char chr = *(*strref)++;
			switch( chr ) {
				case 'a': harbol_string_add_char(str, '\a'); break;
				case 'r': harbol_string_add_char(str, '\r'); break;
				case 'b': harbol_string_add_char(str, '\b'); break;
				case 't': harbol_string_add_char(str, '\t'); break;
				case 'v': harbol_string_add_char(str, '\v'); break;
				case 'n': harbol_string_add_char(str, '\n'); break;
				case 'f': harbol_string_add_char(str, '\f'); break;
				case 's': harbol_string_add_char(str, ' '); break;
				case 'x': {
					const int32_t h = lex_hex_escape_char(*strref, strref);
					if( h<0 ) {
						if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
							harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: bad hex escape character: '%c'. Line: %zu\n", **strref, _g_cfg_err.curr_line);
						return false;
					} else write_utf8_str(str, h);
					break;
				}
				case 'u': case 'U': {
					const int32_t h = lex_unicode_char(*strref, strref, chr=='u' ? sizeof(int16_t) : sizeof(int32_t));
					if( h<0 ) {
						if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
							harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: bad unicode character: '%c'. Line: %zu\n", **strref, _g_cfg_err.curr_line);
						return false;
					} else write_utf8_str(str, h);
					break;
				}
				default: harbol_string_add_char(str, chr);
			}
		}
		else harbol_string_add_char(str, chrval);
	}
	if( **strref==quote )
		(*strref)++;
	
	// Patch, if an empty string was given, we allocate an empty string for the string.
	if( str->cstr==NULL )
		harbol_string_copy_cstr(str, "");
	return **strref != 0;
}
*/

static bool NO_NULL _lex_number(const char **restrict strref, struct HarbolString *const restrict str, enum HarbolCfgType *const typeref)
{
	if( *strref==NULL || **strref==0 )
		return false;
	
	if( **strref=='-' || **strref=='+' )
		harbol_string_add_char(str, *(*strref)++);
	
	if( !is_decimal(**strref) && **strref!='.' ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid initial numeric digit: '%c'. Line: %zu\n", **strref, _g_cfg_err.curr_line);
		return false;
	} else {
		bool is_float = false;
		const char *end = NULL;
		const bool result = lex_c_style_number(*strref, &end, str, &is_float);
		if( !result ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid number. Line: %zu\n", **strref, _g_cfg_err.curr_line);
			return false;
		} else {
			*strref = end;
			*typeref = (is_float) ? HarbolCfgType_Float : HarbolCfgType_Int;
			return str->len > 0;
		}
	}
}

static NO_NULL bool harbol_cfg_parse_section(struct HarbolLinkMap *, const char **);
static NO_NULL bool harbol_cfg_parse_number(struct HarbolLinkMap *, const struct HarbolString *, const char **);

// keyval = <string> [':'] (<value>|<section>) [','] ;
static bool harbol_cfg_parse_key_val(struct HarbolLinkMap *const restrict map, const char **cfgcoderef)
{
	if( *cfgcoderef==NULL ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid config buffer!\n");
		return false;
	} else if( **cfgcoderef==0 || !skip_ws_and_comments(cfgcoderef) )
		return false;
	else if( **cfgcoderef != '"' && **cfgcoderef != '\'' ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: missing beginning quote for key '%c'. Line: %zu\n", **cfgcoderef, _g_cfg_err.curr_line);
		return false;
	}
	
	struct HarbolString keystr = {NULL, 0};
	const bool strresult = lex_c_style_str(*cfgcoderef, cfgcoderef, &keystr);
	if( !strresult ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid string key '%s'. Line: %zu\n", keystr.cstr, _g_cfg_err.curr_line);
		harbol_string_clear(&keystr);
		return false;
	} else if( harbol_linkmap_has_key(map, keystr.cstr) ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: duplicate string key '%s'. Line: %zu\n", keystr.cstr, _g_cfg_err.curr_line);
		harbol_string_clear(&keystr);
		return false;
	}
	skip_ws_and_comments(cfgcoderef);
	
	bool res = false;
	// it's a section!
	if( **cfgcoderef=='{' ) {
		intmax_t *const old = _g_iota.local;
		_g_iota.local = &(intmax_t){0};
		struct HarbolLinkMap *subsection = harbol_linkmap_new(sizeof(struct HarbolVariant));
		res = harbol_cfg_parse_section(subsection, cfgcoderef);
		struct HarbolVariant var = harbol_variant_create(&subsection, sizeof(struct HarbolLinkMap *), HarbolCfgType_Linkmap);
		const bool inserted = harbol_linkmap_insert(map, keystr.cstr, &var);
		if( !inserted )
			harbol_variant_clear(&var, (void(*)(void**))&harbol_cfg_free);
		_g_iota.local = old;
	} else if( **cfgcoderef=='"'||**cfgcoderef=='\'' ) {
		// string value.
		struct HarbolString *str = harbol_string_new("");
		res = lex_c_style_str(*cfgcoderef, cfgcoderef, str);
		if( !res ) {
			if( str==NULL ) {
				if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
					harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: unable to allocate string value. Line: %zu\n", _g_cfg_err.curr_line);
			} else {
				if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
					harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid string value '%s'. Line: %zu\n", str->cstr, _g_cfg_err.curr_line);
			} return false;
		}
		struct HarbolVariant var = harbol_variant_create(&str, sizeof(struct HarbolString *), HarbolCfgType_String);
		harbol_linkmap_insert(map, keystr.cstr, &var);
	} else if( **cfgcoderef=='c' || **cfgcoderef=='v' ) {
		// color/vector value!
		const char valtype = *(*cfgcoderef)++;
		skip_ws_and_comments(cfgcoderef);
		
		if( **cfgcoderef!='[' ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: missing '[' '%c'. Line: %zu\n", **cfgcoderef, _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		(*cfgcoderef)++;
		skip_ws_and_comments(cfgcoderef);
		
		union {
			struct HarbolVec4D vec4d;
			union HarbolColor color;
		} matrix_value = { {0.f, 0.f, 0.f, 0.f} };
		
		size_t iterations = 0;
		while( **cfgcoderef != 0 && **cfgcoderef != ']' ) {
			struct HarbolString numstr = {NULL, 0};
			enum HarbolCfgType type = HarbolCfgType_Null;
			const bool result = _lex_number(cfgcoderef, &numstr, &type);
			if( iterations<4 ) {
				if( valtype=='c' ) {
					switch( iterations ) {
						case 0: matrix_value.color.bytes.r = (uint8_t)strtoul(numstr.cstr, NULL, 0); break;
						case 1: matrix_value.color.bytes.g = (uint8_t)strtoul(numstr.cstr, NULL, 0); break;
						case 2: matrix_value.color.bytes.b = (uint8_t)strtoul(numstr.cstr, NULL, 0); break;
						case 3: matrix_value.color.bytes.a = (uint8_t)strtoul(numstr.cstr, NULL, 0); break;
					}
					iterations++;
				} else {
					/// gotta use `sscanf` for possible hex floats.
					float32_t f = 0;
					switch( iterations ) {
						case 0: sscanf(numstr.cstr, "%" SCNxf32 "", &f); matrix_value.vec4d.x = f; break;
						case 1: sscanf(numstr.cstr, "%" SCNxf32 "", &f); matrix_value.vec4d.y = f; break;
						case 2: sscanf(numstr.cstr, "%" SCNxf32 "", &f); matrix_value.vec4d.z = f; break;
						case 3: sscanf(numstr.cstr, "%" SCNxf32 "", &f); matrix_value.vec4d.w = f; break;
					}
					iterations++;
				}
			}
			harbol_string_clear(&numstr);
			if( !result ) {
				if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
					harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid number in [] array. Line: %zu\n", _g_cfg_err.curr_line);
				harbol_string_clear(&keystr);
				return false;
			}
			skip_ws_and_comments(cfgcoderef);
		}
		if( **cfgcoderef==0 ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: unexpected end of file with missing ending ']'. Line: %zu\n", _g_cfg_err.curr_line);
			return false;
		}
		(*cfgcoderef)++;
		
		struct HarbolVariant var = (valtype=='c') ?
			harbol_variant_create(&matrix_value.color, sizeof(union HarbolColor), HarbolCfgType_Color) : harbol_variant_create(&matrix_value.vec4d, sizeof(struct HarbolVec4D), HarbolCfgType_Vec4D);
		res = harbol_linkmap_insert(map, keystr.cstr, &var);
	} else if( **cfgcoderef=='t' ) {
		// true bool value.
		if( strncmp("true", *cfgcoderef, sizeof("true")-1) ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid word value, only 'true', 'false', 'null', 'Iota', and 'iota' are allowed. Line: %zu\n", _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		*cfgcoderef += sizeof("true") - 1;
		struct HarbolVariant var = harbol_variant_create(&(bool){true}, sizeof(bool), HarbolCfgType_Bool);
		res = harbol_linkmap_insert(map, keystr.cstr, &var);
	} else if( **cfgcoderef=='f' ) {
		// false bool value
		if( strncmp("false", *cfgcoderef, sizeof("false")-1) ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid word value, only 'true', 'false', 'null', 'Iota', and 'iota' are allowed. Line: %zu\n", _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		*cfgcoderef += sizeof("false") - 1;
		struct HarbolVariant *var = harbol_variant_new(&(bool){false}, sizeof(bool), HarbolCfgType_Bool);
		res = harbol_linkmap_insert(map, keystr.cstr, &var);
	} else if( **cfgcoderef=='n' ) {
		// null value.
		if( strncmp("null", *cfgcoderef, sizeof("null")-1) ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid word value, only 'true', 'false', 'null', 'Iota', and 'iota' are allowed. Line: %zu\n", _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		*cfgcoderef += sizeof("null") - 1;
		struct HarbolVariant var = harbol_variant_create(&(char){0}, sizeof(char), HarbolCfgType_Null);
		res = harbol_linkmap_insert(map, keystr.cstr, &var);
	} else if( **cfgcoderef=='I' ) {
		// global iota value.
		if( strncmp("Iota", *cfgcoderef, sizeof("Iota")-1) ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid word value, only 'true', 'false', 'null', 'Iota', and 'iota' are allowed. Line: %zu\n", _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		*cfgcoderef += sizeof("Iota") - 1;
		struct HarbolVariant var = harbol_variant_create(&(intmax_t){_g_iota.global++}, sizeof(intmax_t), HarbolCfgType_Int);
		res = harbol_linkmap_insert(map, keystr.cstr, &var);
	} else if( **cfgcoderef=='i' ) {
		// local iota value.
		if( strncmp("iota", *cfgcoderef, sizeof("iota")-1) ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid word value, only 'true', 'false', 'null', 'Iota', and 'iota' are allowed. Line: %zu\n", _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		*cfgcoderef += sizeof("iota") - 1;
		struct HarbolVariant var = harbol_variant_create(&(intmax_t){(*_g_iota.local)++}, sizeof(intmax_t), HarbolCfgType_Int);
		res = harbol_linkmap_insert(map, keystr.cstr, &var);
	} else if( is_decimal(**cfgcoderef) || **cfgcoderef=='.' || **cfgcoderef=='-' || **cfgcoderef=='+' ) {
		// numeric value.
		res = harbol_cfg_parse_number(map, &keystr, cfgcoderef);
	} else if( **cfgcoderef=='[' ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: array bracket missing 'c' or 'v' tag. Line: %zu\n", _g_cfg_err.curr_line);
		harbol_string_clear(&keystr);
		return false;
	} else {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: unknown character detected '%c'. Line: %zu\n", **cfgcoderef, _g_cfg_err.curr_line);
		res = false;
	}
	harbol_string_clear(&keystr);
	skip_ws_and_comments(cfgcoderef);
	return res;
}

static bool harbol_cfg_parse_number(struct HarbolLinkMap *const restrict map, const struct HarbolString *const restrict key, const char **cfgcoderef)
{
	struct HarbolString numstr = {NULL, 0};
	enum HarbolCfgType type = HarbolCfgType_Null;
	const bool result = _lex_number(cfgcoderef, &numstr, &type);
	if( !result ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid number. Line: %zu\n", _g_cfg_err.curr_line);
		harbol_string_clear(&numstr);
		return result;
	} else {
		struct HarbolVariant var = { 0 };
		if( type==HarbolCfgType_Float ) {
			floatmax_t f = 0;
			sscanf(numstr.cstr, "%" SCNxfMAX "", &f);
			var = harbol_variant_create(&f, sizeof(floatmax_t), type);
		} else {
			var = harbol_variant_create(&(intmax_t){strtoll(numstr.cstr, NULL, 0)}, sizeof(intmax_t), HarbolCfgType_Int);
		}
		harbol_string_clear(&numstr);
		return harbol_linkmap_insert(map, key->cstr, &var);
	}
}

// section = '{' <keyval> '}' ;
static bool harbol_cfg_parse_section(struct HarbolLinkMap *const restrict map, const char **cfgcoderef)
{
	if( **cfgcoderef!='{' ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: missing '{' but got '%c' for section. Line: %zu\n", **cfgcoderef, _g_cfg_err.curr_line);
		return false;
	}
	(*cfgcoderef)++;
	skip_ws_and_comments(cfgcoderef);
	
	while( **cfgcoderef != 0 && **cfgcoderef != '}' ) {
		const bool res = harbol_cfg_parse_key_val(map, cfgcoderef);
		if( !res )
			return false;
	}
	if( **cfgcoderef==0 ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: unexpected end of file with missing '}' for section. Line: %zu\n", _g_cfg_err.curr_line);
		return false;
	}
	(*cfgcoderef)++;
	return true;
}


HARBOL_EXPORT struct HarbolLinkMap *harbol_cfg_parse_file(const char filename[restrict static 1])
{
	FILE *restrict cfgfile = fopen(filename, "r");
	if( cfgfile==NULL ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: unable to find file '%s'.\n", filename);
		return NULL;
	} else {
		struct HarbolString cfg = {NULL, 0};
		const bool read_result = harbol_string_read_file(&cfg, cfgfile);
		fclose(cfgfile);
		
		if( !read_result ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: failed to read file '%s' into a string.\n", filename);
			return NULL;
		} else {
			struct HarbolLinkMap *const restrict objs = harbol_cfg_parse_cstr(cfg.cstr);
			harbol_string_clear(&cfg);
			return objs;
		}
	}
}


HARBOL_EXPORT struct HarbolLinkMap *harbol_cfg_parse_cstr(const char cfgcode[])
{
	_g_cfg_err.curr_line = 1;
	const char *iter = cfgcode;
	struct HarbolLinkMap *objs = harbol_linkmap_new(sizeof(struct HarbolVariant));
	if( objs==NULL )
		return NULL;
	else {
		_g_iota.global = 0;
		_g_iota.local = &(intmax_t){0};
		while( harbol_cfg_parse_key_val(objs, &iter) );
		if( _g_cfg_err.count > 0 ) {
			for( uindex_t i=0; i<_g_cfg_err.count; i++ ) {
				fputs(_g_cfg_err.errs[i].cstr, stderr);
				harbol_string_clear(&_g_cfg_err.errs[i]);
			}
			_g_cfg_err.count = 0;
		}
		return objs;
	}
}

union ConfigVal {
	uint8_t *restrict data;
	struct HarbolLinkMap **restrict section;
	struct HarbolString **restrict str;
	floatmax_t *restrict f;
	intmax_t *restrict i;
	bool *restrict b;
	union HarbolColor *restrict c;
	struct HarbolVec4D *restrict v;
};

static void __harbol_cfgkey_del(struct HarbolVariant *const var)
{
	union ConfigVal cv = {var->data};
	switch( var->tag ) {
		case HarbolCfgType_Linkmap:
			harbol_cfg_free(cv.section);
			harbol_variant_clear(var, NULL);
			break;
		case HarbolCfgType_String:
			harbol_string_free(cv.str);
			harbol_variant_clear(var, NULL);
			break;
		default:
			harbol_variant_clear(var, NULL);
	}
}

HARBOL_EXPORT bool harbol_cfg_free(struct HarbolLinkMap **mapref)
{
	if( *mapref==NULL )
		return false;
	else {
		struct HarbolKeyVal **const end = harbol_linkmap_get_iter_end_count(*mapref);
		for( struct HarbolKeyVal **iter = harbol_linkmap_get_iter(*mapref); iter && iter<end; iter++ )
			__harbol_cfgkey_del((struct HarbolVariant *)(*iter)->data);
		
		harbol_linkmap_free(mapref, NULL);
		return *mapref==NULL;
	}
}

static inline NO_NULL void __concat_tabs(struct HarbolString *const str, const size_t tabs)
{
	for( uindex_t i=0; i<tabs; i++ )
		harbol_string_add_cstr(str, "\t");
}

HARBOL_EXPORT struct HarbolString harbol_cfg_to_str(const struct HarbolLinkMap *const map)
{
	static size_t tabs = 0;
	struct HarbolString str = harbol_string_create("");
	for( uindex_t i=0; i<map->vec.count; i++ ) {
		struct HarbolKeyVal **const iter = harbol_vector_get(&map->vec, i);
		const struct HarbolVariant *var = (const struct HarbolVariant *)(*iter)->data;
		
		const union ConfigVal cv = { var->data };
		// using double pointer iterators as we need the key.
		__concat_tabs(&str, tabs);
		harbol_string_add_format(&str, "\"%s\": ", (*iter)->key.cstr);
		switch( var->tag ) {
			case HarbolCfgType_Null:
				harbol_string_add_cstr(&str, "null\n");
				break;
			case HarbolCfgType_Linkmap: {
				harbol_string_add_cstr(&str, "{\n");
				tabs++;
				struct HarbolString inner_str = harbol_cfg_to_str(*cv.section);
				harbol_string_add_format(&str, "%s", inner_str.cstr);
				__concat_tabs(&str, --tabs);
				harbol_string_add_cstr(&str, "}\n");
				harbol_string_clear(&inner_str);
				break;
			}
			case HarbolCfgType_String:
				harbol_string_add_format(&str, "\"%s\"\n", (*cv.str)->cstr);
				break;
			case HarbolCfgType_Float:
				harbol_string_add_format(&str, "%" PRIfMAX "\n", *cv.f);
				break;
			case HarbolCfgType_Int:
				harbol_string_add_format(&str, "%" PRIiMAX "\n", *cv.i);
				break;
			case HarbolCfgType_Bool:
				harbol_string_add_cstr(&str, *cv.b ? "true\n" : "false\n");
				break;
			case HarbolCfgType_Color:
				harbol_string_add_format(&str, "c[ %u, %u, %u, %u ]\n", cv.c->bytes.r, cv.c->bytes.g, cv.c->bytes.b, cv.c->bytes.a);
				break;
			case HarbolCfgType_Vec4D:
				harbol_string_add_format(&str, "v[ %" PRIf32 ", %" PRIf32 ", %" PRIf32 ", %" PRIf32 " ]\n", cv.v->x, cv.v->y, cv.v->z, cv.v->w);
				break;
		}
	}
	return str;
}

static NO_NULL bool harbol_cfg_parse_target_path(const char key[static 1], struct HarbolString *const restrict str)
{
	// parse something like: "root.section1.section2.section3./.dotsection"
	const char *iter = key;
	/*
		iterate to the null terminator and then work backwards to the last dot.
		ughhh too many while loops lmao.
	*/
	iter += strlen(key) - 1;
	while( iter != key ) {
		// Patch: allow keys to use dot without interfering with dot path.
		// check if we hit a dot.
		if( *iter=='.' ) {
			// if we hit a dot, check if the previous char is an "escape" char.
			if( iter[-1]=='/' || iter[-1]=='\\' )
				iter--;
			else {
				iter++;
				break;
			}
		} else iter--;
	}
	// now we save the target section and then use the resulting string.
	while( *iter != 0 ) {
		if( *iter=='/' ) {
			iter++;
			continue;
		}
		else harbol_string_add_char(str, *iter++);
	}
	return str->len > 0;
}

static NO_NULL struct HarbolVariant *__get_var(struct HarbolLinkMap *const restrict cfgmap, const char key[static 1])
{
	/* first check if we're getting a singular value OR we iterate through a sectional path. */
	const char *dot = strchr(key, '.');
	// Patch: dot and escaped dot glitching out the hashmap hashing...
	if( dot==NULL || (dot>key && (dot[-1] == '/' || dot[-1] == '\\')) ) {
		struct HarbolVariant *const restrict var = harbol_linkmap_key_get(cfgmap, key);
		return( var==NULL || var->tag==HarbolCfgType_Null ) ? NULL : var;
	}
	/* ok, not a singular value, iterate to the specific linkmap section then. */
	else {
		// parse the target key first.
		const char *iter = key;
		struct HarbolString
			sectionstr = {NULL, 0},
			targetstr = {NULL, 0}
		;
		harbol_cfg_parse_target_path(key, &targetstr);
		struct HarbolLinkMap *restrict itermap = cfgmap;
		struct HarbolVariant *restrict var = NULL;
		
		while( itermap != NULL ) {
			harbol_string_clear(&sectionstr);
			// Patch: allow keys to use dot without interfering with dot path.
			while( *iter != 0 ) {
				if( (*iter=='/' || *iter=='\\') && iter[1] && iter[1]=='.' ) {
					iter++;
					harbol_string_add_char(&sectionstr, *iter++);
				} else if( *iter=='.' ) {
					iter++;
					break;
				}
				else harbol_string_add_char(&sectionstr, *iter++);
			}
			var = harbol_linkmap_key_get(itermap, sectionstr.cstr);
			if( var==NULL || !harbol_string_cmpstr(&sectionstr, &targetstr) )
				break;
			else if( var->tag==HarbolCfgType_Linkmap )
				itermap = *(struct HarbolLinkMap **)var->data;
		}
		harbol_string_clear(&sectionstr);
		harbol_string_clear(&targetstr);
		return var;
	}
}

HARBOL_EXPORT struct HarbolLinkMap *harbol_cfg_get_section(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->tag != HarbolCfgType_Linkmap ) ? NULL : *(struct HarbolLinkMap **)var->data;
}

HARBOL_EXPORT char *harbol_cfg_get_cstr(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->tag != HarbolCfgType_String ) ? NULL : (*(struct HarbolString **)var->data)->cstr;
}

HARBOL_EXPORT struct HarbolString *harbol_cfg_get_str(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->tag != HarbolCfgType_String ) ? NULL : *(struct HarbolString **)var->data;
}

HARBOL_EXPORT floatmax_t *harbol_cfg_get_float(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->tag != HarbolCfgType_Float ) ? NULL : (floatmax_t *)var->data;
}

HARBOL_EXPORT intmax_t *harbol_cfg_get_int(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->tag != HarbolCfgType_Int ) ? NULL : (intmax_t *)var->data;
}

HARBOL_EXPORT bool *harbol_cfg_get_bool(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->tag != HarbolCfgType_Bool ) ? NULL : (bool *)var->data;
}

HARBOL_EXPORT union HarbolColor *harbol_cfg_get_color(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->tag != HarbolCfgType_Color ) ? NULL : (union HarbolColor *)var->data;
}


HARBOL_EXPORT struct HarbolVec4D *harbol_cfg_get_vec4D(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->tag != HarbolCfgType_Vec4D ) ? NULL : (struct HarbolVec4D *)var->data;
}

HARBOL_EXPORT enum HarbolCfgType harbol_cfg_get_type(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL ) ? -1 : var->tag;
}

HARBOL_EXPORT bool harbol_cfg_set_str(struct HarbolLinkMap *const restrict cfgmap, const char keypath[restrict static 1], const struct HarbolString str, const bool override_convert)
{
	return harbol_cfg_set_cstr(cfgmap, keypath, str.cstr, override_convert);
}

HARBOL_EXPORT bool harbol_cfg_set_cstr(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], const char cstr[restrict static 1], const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->tag != HarbolCfgType_String ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			struct HarbolString *str = harbol_string_new(cstr);
			*var = harbol_variant_create(&str, sizeof(struct HarbolString *), HarbolCfgType_String);
			return true;
		}
		else return false;
	} else {
		harbol_string_copy_cstr(*(struct HarbolString **)var->data, cstr);
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_float(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], floatmax_t val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->tag != HarbolCfgType_Float ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(floatmax_t), HarbolCfgType_Float);
			return true;
		}
		else return false;
	} else {
		*(floatmax_t *)var->data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_int(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], intmax_t val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->tag != HarbolCfgType_Int ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(intmax_t), HarbolCfgType_Int);
			return true;
		}
		else return false;
	} else {
		*(intmax_t *)var->data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_bool(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], bool val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->tag != HarbolCfgType_Bool ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(bool), HarbolCfgType_Bool);
			return true;
		}
		else return false;
	} else {
		*(bool *)var->data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_color(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], union HarbolColor val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->tag != HarbolCfgType_Color ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(union HarbolColor), HarbolCfgType_Color);
			return true;
		}
		else return false;
	} else {
		*(union HarbolColor *)var->data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_vec4D(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], struct HarbolVec4D val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL ) {
		return false;
	} else if( var->tag != HarbolCfgType_Vec4D ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(struct HarbolVec4D), HarbolCfgType_Vec4D);
			return true;
		}
		else return false;
	} else {
		*(struct HarbolVec4D *)var->data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_to_null(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL ) {
		return false;
	} else {
		__harbol_cfgkey_del(var);
		*var = harbol_variant_create(&(char){0}, sizeof(char), HarbolCfgType_Null);
		return true;
	}
}

static inline NO_NULL void __write_tabs(FILE *const file, const size_t tabs)
{
	for( uindex_t i=0; i<tabs; i++ )
		fputs("\t", file);
}

static NO_NULL bool __harbol_cfg_build_file(const struct HarbolLinkMap *const map, FILE *const file, const size_t tabs)
{
	const struct HarbolKeyVal **const end = harbol_linkmap_get_iter_end_count(map);
	for( const struct HarbolKeyVal **iter = harbol_linkmap_get_iter(map); iter && iter<end; iter++ ) {
		const struct HarbolVariant *v = (const struct HarbolVariant *)(*iter)->data;
		const int32_t type = v->tag;
		__write_tabs(file, tabs);
		// using double pointer iterators as we need the key.
		fprintf(file, "\"%s\": ", (*iter)->key.cstr);
		
		const union ConfigVal cv = {v->data};
		switch( type ) {
			case HarbolCfgType_Null:
				fputs("null\n", file); break;
			case HarbolCfgType_Linkmap:
				fputs("{\n", file);
				__harbol_cfg_build_file(*cv.section, file, tabs+1);
				__write_tabs(file, tabs);
				fputs("}\n", file);
				break;
			
			case HarbolCfgType_String:
				fprintf(file, "\"%s\"\n", (*cv.str)->cstr); break;
			case HarbolCfgType_Float:
				fprintf(file, "%" PRIfMAX "\n", *cv.f); break;
			case HarbolCfgType_Int:
				fprintf(file, "%" PRIiMAX "\n", *cv.i); break;
			case HarbolCfgType_Bool:
				fprintf(file, "%s\n", (*cv.b) ? "true" : "false"); break;
			case HarbolCfgType_Color:
				fprintf(file, "c[ %u, %u, %u, %u ]\n", cv.c->bytes.r, cv.c->bytes.g, cv.c->bytes.b, cv.c->bytes.a); break;
			case HarbolCfgType_Vec4D:
				fprintf(file, "v[ %" PRIf32 ", %" PRIf32 ", %" PRIf32 ", %" PRIf32 " ]\n", cv.v->x, cv.v->y, cv.v->z, cv.v->w); break;
		}
	}
	return true;
}

HARBOL_EXPORT bool harbol_cfg_build_file(const struct HarbolLinkMap *const restrict cfg, const char filename[restrict static 1], const bool overwrite)
{
	FILE *restrict cfgfile = fopen(filename, overwrite ? "w+" : "a+");
	if( cfgfile==NULL ) {
		fputs("harbol_cfg_build_file :: unable to create file.\n", stderr);
		return false;
	}
	const bool result = __harbol_cfg_build_file(cfg, cfgfile, 0);
	fclose(cfgfile), cfgfile=NULL;
	return result;
}
