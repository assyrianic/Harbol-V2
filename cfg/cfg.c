#include "cfg.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

/* CFG Parser in EBNF grammar
	keyval = <string> [':'] (<value>|<section>) [','] ;
	section = '{' <keyval> '}' ;
	value = <string> | <number> | <color> | "true" | "false" | "null" ;
	matrix = '[' <number> [','] [<number>] [','] [<number>] [','] [<number>] ']' ;
	color = 'c' <matrix> ;
	vecf = 'v' <matrix> ;
	string = '"' chars '"' | "'" chars "'" ;
*/

#ifndef HARBOL_CFG_ERR_STK_SIZE
#	define HARBOL_CFG_ERR_STK_SIZE 20
#endif

static struct {
	struct HarbolString errs[HARBOL_CFG_ERR_STK_SIZE];
	size_t count, curr_line;
} _g_cfg_err;


static NO_NULL void skip_single_comment(const char **strref)
{
	if( *strref==NULL || **strref==0 )
		return;
	else for(; **strref != '\n'; (*strref)++ );
}

static NO_NULL void skip_multiline_comment(const char **strref)
{
	if( *strref==NULL || **strref==0 )
		return;
	else {
		// skip past '/' && '*'
		*strref += 2;
		do {
			if( **strref==0 || (*strref)[1]==0 )
				break;
			if( **strref=='\n' )
				_g_cfg_err.curr_line++;
			(*strref)++;
		} while( !(**strref=='*' && (*strref)[1]=='/') );
		if( **strref && (*strref)[1] )
			*strref += 2;
	}
}

static NO_NULL bool skip_whitespace(const char **strref)
{
	if( *strref==NULL || **strref==0 )
		return false;
	else {
		while( **strref && is_whitespace(**strref) ) {
			if( **strref=='\n' )
				_g_cfg_err.curr_line++;
			(*strref)++;
		}
		return **strref != 0;
	}
}

static NO_NULL bool skip_ws_and_comments(const char **strref)
{
	if( *strref==NULL || **strref==0 ) {
		return false;
	} else {
		while( **strref && (is_whitespace(**strref) || // white space
				**strref=='#' || (**strref=='/' && (*strref)[1]=='/') || // single line comment
				(**strref=='/' && (*strref)[1]=='*') || // multi-line comment
				**strref==':' || **strref==',') ) // delimiters.
		{
			if( is_whitespace(**strref) )
				skip_whitespace(strref);
			else if( **strref=='#' || (**strref=='/' && (*strref)[1]=='/') ) {
				skip_single_comment(strref);
				_g_cfg_err.curr_line++;
			} else if( **strref=='/' && (*strref)[1]=='*' )
				skip_multiline_comment(strref);
			else if( **strref==':' || **strref==',' )
				(*strref)++;
		}
		return **strref != 0;
	}
}

static NO_NULL int32_t _lex_hex_escape_char(const char **restrict strref)
{
	int32_t r = 0;
	if( !is_hex(**strref) ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: \\x escape hex with no digits! '%c'. Line: %zu\n", **strref, _g_cfg_err.curr_line);
	} else {
		for(; **strref; (*strref)++ ) {
			const char c = **strref;
			if( c>='0' && c<='9' )
				r = (r << 4) | (c - '0');
			else if( c>='a' && c<='f' )
				r = (r << 4) | (c - 'a' + 10);
			else if( c>='A' && c<='F' )
				r = (r << 4) | (c - 'A' + 10);
			else return r;
		}
	}
	return r;
}


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
	while( **strref && **strref != quote ) {
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
				case 'x': harbol_string_add_char(str, (char)_lex_hex_escape_char(strref)); break;
				default: harbol_string_add_char(str, chr);
			}
		}
		else harbol_string_add_char(str, chrval);
	}
	if( **strref==quote )
		(*strref)++;
	
	// Patch, if an empty string was given, we allocate an empty string for the string.
	if( str->CStr==NULL )
		harbol_string_copy_cstr(str, "");
	return **strref != 0;
}

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
	} else if( **strref=='0' ) {
		harbol_string_add_char(str, *(*strref)++);
		const char numtype = *(*strref)++;
		harbol_string_add_char(str, numtype);
		*typeref = HarbolCfgType_Int;
		switch( numtype ) {
			case 'X': case 'x': // hex
				harbol_string_add_char(str, *(*strref)++);
				while( is_hex(**strref) )
					harbol_string_add_char(str, *(*strref)++);
				break;
			case '.': // float
				*typeref = HarbolCfgType_Float;
				harbol_string_add_char(str, *(*strref)++);
				while( is_decimal(**strref) || **strref=='e' || **strref=='E' || **strref=='f' || **strref=='F' )
					harbol_string_add_char(str, *(*strref)++);
				break;
			default: // octal
				while( is_octal(**strref) )
					harbol_string_add_char(str, *(*strref)++);
		}
	}
	else if( is_decimal(**strref) ) { // numeric value. Check if float possibly.
		*typeref = HarbolCfgType_Int;
		while( is_decimal(**strref) )
			harbol_string_add_char(str, *(*strref)++);
		
		if( **strref=='.' ) { // definitely float value.
			*typeref = HarbolCfgType_Float;
			harbol_string_add_char(str, *(*strref)++);
			while( is_decimal(**strref) || **strref=='e' || **strref=='E' || **strref=='f' || **strref=='F' )
				harbol_string_add_char(str, *(*strref)++);
		}
	}
	else if( **strref=='.' ) { // float value.
		*typeref = HarbolCfgType_Float;
		harbol_string_add_char(str, *(*strref)++);
		while( is_decimal(**strref) || **strref=='e' || **strref=='E' )
			harbol_string_add_char(str, *(*strref)++);
	}
	return str->Len > 0;
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
	} else if( !**cfgcoderef || !skip_ws_and_comments(cfgcoderef) )
		return false;
	else if( **cfgcoderef!='"' && **cfgcoderef!='\'' ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: missing beginning quote for key '%c'. Line: %zu\n", **cfgcoderef, _g_cfg_err.curr_line);
		return false;
	}
	
	struct HarbolString keystr = {NULL, 0};
	const bool strresult = _lex_string(cfgcoderef, &keystr);
	if( !strresult ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid string key '%s'. Line: %zu\n", keystr.CStr, _g_cfg_err.curr_line);
		harbol_string_clear(&keystr);
		return false;
	} else if( harbol_linkmap_has_key(map, keystr.CStr) ) {
		if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
			harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: duplicate string key '%s'. Line: %zu\n", keystr.CStr, _g_cfg_err.curr_line);
		harbol_string_clear(&keystr);
		return false;
	}
	skip_ws_and_comments(cfgcoderef);
	
	bool res = false;
	// it's a section!
	if( **cfgcoderef=='{' ) {
		struct HarbolLinkMap *subsection = harbol_linkmap_new(sizeof(struct HarbolVariant));
		res = harbol_cfg_parse_section(subsection, cfgcoderef);
		struct HarbolVariant var = harbol_variant_create(&subsection, sizeof(struct HarbolLinkMap *), HarbolCfgType_Linkmap);
		const bool inserted = harbol_linkmap_insert(map, keystr.CStr, &var);
		if( !inserted )
			harbol_variant_clear(&var, (void(*)(void**))&harbol_cfg_free);
	} else if( **cfgcoderef=='"'||**cfgcoderef=='\'' ) {
		// string value.
		struct HarbolString *str = harbol_string_new("");
		res = _lex_string(cfgcoderef, str);
		if( !res ) {
			if( str==NULL ) {
				if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
					harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: unable to allocate string value. Line: %zu\n", _g_cfg_err.curr_line);
			} else {
				if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
					harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid string value '%s'. Line: %zu\n", str->CStr, _g_cfg_err.curr_line);
			} return false;
		}
		struct HarbolVariant var = harbol_variant_create(&str, sizeof(struct HarbolString *), HarbolCfgType_String);
		harbol_linkmap_insert(map, keystr.CStr, &var);
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
		} matrix_value;
		memset(&matrix_value, 0, sizeof matrix_value);
		
		size_t iterations = 0;
		while( **cfgcoderef && **cfgcoderef != ']' ) {
			struct HarbolString numstr = {NULL, 0};
			enum HarbolCfgType type = HarbolCfgType_Null;
			const bool result = _lex_number(cfgcoderef, &numstr, &type);
			if( iterations<4 ) {
				if( valtype=='c' ) {
					switch( iterations ) {
						case 0: matrix_value.color.Bytes.R = (uint8_t)strtoul(numstr.CStr, NULL, 0); break;
						case 1: matrix_value.color.Bytes.G = (uint8_t)strtoul(numstr.CStr, NULL, 0); break;
						case 2: matrix_value.color.Bytes.B = (uint8_t)strtoul(numstr.CStr, NULL, 0); break;
						case 3: matrix_value.color.Bytes.A = (uint8_t)strtoul(numstr.CStr, NULL, 0); break;
					}
					iterations++;
				} else {
					switch( iterations ) {
						case 0: matrix_value.vec4d.X = (float)strtof(numstr.CStr, NULL); break;
						case 1: matrix_value.vec4d.Y = (float)strtof(numstr.CStr, NULL); break;
						case 2: matrix_value.vec4d.Z = (float)strtof(numstr.CStr, NULL); break;
						case 3: matrix_value.vec4d.W = (float)strtof(numstr.CStr, NULL); break;
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
		if( !**cfgcoderef ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: unexpected end of file with missing ending ']'. Line: %zu\n", _g_cfg_err.curr_line);
			return false;
		}
		(*cfgcoderef)++;
		
		struct HarbolVariant var = (valtype=='c') ?
			harbol_variant_create(&matrix_value.color, sizeof(union HarbolColor), HarbolCfgType_Color) : harbol_variant_create(&matrix_value.vec4d, sizeof(struct HarbolVec4D), HarbolCfgType_Vec4D);
		res = harbol_linkmap_insert(map, keystr.CStr, &var);
	} else if( **cfgcoderef=='t' ) {
		// true bool value.
		if( strncmp("true", *cfgcoderef, sizeof("true")-1) ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid word value, only 'true', 'false' or 'null' are allowed. Line: %zu\n", _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		*cfgcoderef += sizeof("true") - 1;
		struct HarbolVariant var = harbol_variant_create(&(bool){true}, sizeof(bool), HarbolCfgType_Bool);
		res = harbol_linkmap_insert(map, keystr.CStr, &var);
	} else if( **cfgcoderef=='f' ) {
		// false bool value
		if( strncmp("false", *cfgcoderef, sizeof("false")-1) ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid word value, only 'true', 'false' or 'null' are allowed. Line: %zu\n", _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		*cfgcoderef += sizeof("false") - 1;
		struct HarbolVariant *var = harbol_variant_new(&(bool){false}, sizeof(bool), HarbolCfgType_Bool);
		res = harbol_linkmap_insert(map, keystr.CStr, &var);
	} else if( **cfgcoderef=='n' ) {
		// null value.
		if( strncmp("null", *cfgcoderef, sizeof("null")-1) ) {
			if( _g_cfg_err.count < HARBOL_CFG_ERR_STK_SIZE )
				harbol_string_format(&_g_cfg_err.errs[_g_cfg_err.count++], "Harbol Config Parser :: invalid word value, only 'true', 'false' or 'null' are allowed. Line: %zu\n", _g_cfg_err.curr_line);
			harbol_string_clear(&keystr);
			return false;
		}
		*cfgcoderef += sizeof("null") - 1;
		struct HarbolVariant var = harbol_variant_create(&(char){0}, sizeof(char), HarbolCfgType_Null);
		res = harbol_linkmap_insert(map, keystr.CStr, &var);
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
		struct HarbolVariant var = ( type==HarbolCfgType_Float ) ?
			harbol_variant_create(&(floatmax_t){strtofmax(numstr.CStr, NULL)}, sizeof(floatmax_t), HarbolCfgType_Float) : harbol_variant_create(&(intmax_t){strtoll(numstr.CStr, NULL, 0)}, sizeof(intmax_t), HarbolCfgType_Int);
		harbol_string_clear(&numstr);
		return harbol_linkmap_insert(map, key->CStr, &var);
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
	
	while( **cfgcoderef && **cfgcoderef != '}' ) {
		const bool res = harbol_cfg_parse_key_val(map, cfgcoderef);
		if( !res )
			return false;
	}
	if( !**cfgcoderef ) {
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
			struct HarbolLinkMap *const restrict objs = harbol_cfg_parse_cstr(cfg.CStr);
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
		while( harbol_cfg_parse_key_val(objs, &iter) );
		if( _g_cfg_err.count > 0 ) {
			for( uindex_t i=0; i<_g_cfg_err.count; i++ ) {
				fputs(_g_cfg_err.errs[i].CStr, stderr);
				harbol_string_clear(&_g_cfg_err.errs[i]);
			}
			_g_cfg_err.count = 0;
		}
		return objs;
	}
}

static void __harbol_cfgkey_del(struct HarbolVariant *const var)
{
	switch( var->Type ) {
		case HarbolCfgType_Linkmap:
			harbol_cfg_free((struct HarbolLinkMap**)var->Data);
			harbol_variant_clear(var, NULL);
			break;
		case HarbolCfgType_String:
			harbol_string_free((struct HarbolString**)var->Data);
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
			__harbol_cfgkey_del((struct HarbolVariant *)(*iter)->Data);
		
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
	for( uindex_t i=0; i<map->Vec.Count; i++ ) {
		struct HarbolKeyVal **const iter = harbol_vector_get(&map->Vec, i);
		const struct HarbolVariant *var = (const struct HarbolVariant *)(*iter)->Data;
		// using double pointer iterators as we need the key.
		__concat_tabs(&str, tabs);
		harbol_string_add_format(&str, "\"%s\": ", (*iter)->Key.CStr);
		switch( var->Type ) {
			case HarbolCfgType_Null:
				harbol_string_add_cstr(&str, "null\n");
				break;
			case HarbolCfgType_Linkmap: {
				harbol_string_add_cstr(&str, "{\n");
				tabs++;
				struct HarbolString inner_str = harbol_cfg_to_str(*(const struct HarbolLinkMap **)var->Data);
				harbol_string_add_format(&str, "%s", inner_str.CStr);
				__concat_tabs(&str, --tabs);
				harbol_string_add_cstr(&str, "}\n");
				harbol_string_clear(&inner_str);
				break;
			}
			case HarbolCfgType_String:
				harbol_string_add_format(&str, "\"%s\"\n", ((*(const struct HarbolString **)var->Data))->CStr);
				break;
			case HarbolCfgType_Float:
				harbol_string_add_format(&str, "%" PRIfMAX "\n", *((const floatmax_t *)var->Data));
				break;
			case HarbolCfgType_Int:
				harbol_string_add_format(&str, "%" PRIiMAX "\n", *((const intmax_t *)var->Data));
				break;
			case HarbolCfgType_Bool:
				harbol_string_add_cstr(&str, *((bool *)var->Data) ? "true\n" : "false\n");
				break;
			case HarbolCfgType_Color: {
				const struct { uint8_t r,g,b,a; } *color = (const void*)var->Data;
				harbol_string_add_format(&str, "c[ %u, %u, %u, %u ]\n", color->r, color->g, color->b, color->a);
				break;
			}
			case HarbolCfgType_Vec4D: {
				const struct { float x,y,z,w; } *vec4 = (const void*)var->Data;
				harbol_string_add_format(&str, "v[ %f, %f, %f, %f ]\n", vec4->x, vec4->y, vec4->z, vec4->w);
				break;
			}
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
	while( *iter ) {
		if( *iter=='/' ) {
			iter++;
			continue;
		}
		else harbol_string_add_char(str, *iter++);
	}
	return str->Len > 0;
}

static NO_NULL struct HarbolVariant *__get_var(struct HarbolLinkMap *const restrict cfgmap, const char key[static 1])
{
	/* first check if we're getting a singular value OR we iterate through a sectional path. */
	const char *dot = strchr(key, '.');
	// Patch: dot and escaped dot glitching out the hashmap hashing...
	if( dot==NULL || (dot>key && (dot[-1] == '/' || dot[-1] == '\\')) ) {
		struct HarbolVariant *const restrict var = harbol_linkmap_key_get(cfgmap, key);
		return( var==NULL || var->Type==HarbolCfgType_Null ) ? NULL : var;
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
			while( *iter ) {
				if( (*iter=='/' || *iter=='\\') && iter[1] && iter[1]=='.' ) {
					iter++;
					harbol_string_add_char(&sectionstr, *iter++);
				} else if( *iter=='.' ) {
					iter++;
					break;
				}
				else harbol_string_add_char(&sectionstr, *iter++);
			}
			var = harbol_linkmap_key_get(itermap, sectionstr.CStr);
			if( var==NULL || !harbol_string_cmpstr(&sectionstr, &targetstr) )
				break;
			else if( var->Type==HarbolCfgType_Linkmap )
				itermap = *(struct HarbolLinkMap **)var->Data;
		}
		harbol_string_clear(&sectionstr);
		harbol_string_clear(&targetstr);
		return var;
	}
}

HARBOL_EXPORT struct HarbolLinkMap *harbol_cfg_get_section(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->Type != HarbolCfgType_Linkmap ) ? NULL : *(struct HarbolLinkMap **)var->Data;
}

HARBOL_EXPORT char *harbol_cfg_get_cstr(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->Type != HarbolCfgType_String ) ? NULL : (*(struct HarbolString **)var->Data)->CStr;
}

HARBOL_EXPORT struct HarbolString *harbol_cfg_get_str(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->Type != HarbolCfgType_String ) ? NULL : *(struct HarbolString **)var->Data;
}

HARBOL_EXPORT floatmax_t *harbol_cfg_get_float(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->Type != HarbolCfgType_Float ) ? NULL : (floatmax_t *)var->Data;
}

HARBOL_EXPORT intmax_t *harbol_cfg_get_int(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->Type != HarbolCfgType_Int ) ? NULL : (intmax_t *)var->Data;
}

HARBOL_EXPORT bool *harbol_cfg_get_bool(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->Type != HarbolCfgType_Bool ) ? NULL : (bool *)var->Data;
}

HARBOL_EXPORT union HarbolColor *harbol_cfg_get_color(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->Type != HarbolCfgType_Color ) ? NULL : (union HarbolColor *)var->Data;
}


HARBOL_EXPORT struct HarbolVec4D *harbol_cfg_get_vec4D(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL || var->Type != HarbolCfgType_Vec4D ) ? NULL : (struct HarbolVec4D *)var->Data;
}

HARBOL_EXPORT enum HarbolCfgType harbol_cfg_get_type(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1])
{
	const struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	return( var==NULL ) ? -1 : var->Type;
}

HARBOL_EXPORT bool harbol_cfg_set_str(struct HarbolLinkMap *const restrict cfgmap, const char keypath[restrict static 1], const struct HarbolString str, const bool override_convert)
{
	return harbol_cfg_set_cstr(cfgmap, keypath, str.CStr, override_convert);
}

HARBOL_EXPORT bool harbol_cfg_set_cstr(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], const char cstr[restrict static 1], const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->Type != HarbolCfgType_String ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			struct HarbolString *str = harbol_string_new(cstr);
			*var = harbol_variant_create(&str, sizeof(struct HarbolString *), HarbolCfgType_String);
			return true;
		}
		else return false;
	} else {
		harbol_string_copy_cstr(*(struct HarbolString **)var->Data, cstr);
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_float(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], floatmax_t val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->Type != HarbolCfgType_Float ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(floatmax_t), HarbolCfgType_Float);
			return true;
		}
		else return false;
	} else {
		*(floatmax_t *)var->Data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_int(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], intmax_t val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->Type != HarbolCfgType_Int ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(intmax_t), HarbolCfgType_Int);
			return true;
		}
		else return false;
	} else {
		*(intmax_t *)var->Data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_bool(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], bool val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->Type != HarbolCfgType_Bool ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(bool), HarbolCfgType_Bool);
			return true;
		}
		else return false;
	} else {
		*(bool *)var->Data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_color(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], union HarbolColor val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL )
		return false;
	else if( var->Type != HarbolCfgType_Color ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(union HarbolColor), HarbolCfgType_Color);
			return true;
		}
		else return false;
	} else {
		*(union HarbolColor *)var->Data = val;
		return true;
	}
}

HARBOL_EXPORT bool harbol_cfg_set_vec4D(struct HarbolLinkMap *const restrict cfgmap, const char key[restrict static 1], struct HarbolVec4D val, const bool override_convert)
{
	struct HarbolVariant *const restrict var = __get_var(cfgmap, key);
	if( var==NULL ) {
		return false;
	} else if( var->Type != HarbolCfgType_Vec4D ) {
		if( override_convert ) {
			__harbol_cfgkey_del(var);
			*var = harbol_variant_create(&val, sizeof(struct HarbolVec4D), HarbolCfgType_Vec4D);
			return true;
		}
		else return false;
	} else {
		*(struct HarbolVec4D *)var->Data = val;
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
		const struct HarbolVariant *v = (const struct HarbolVariant *)(*iter)->Data;
		const int32_t type = v->Type;
		__write_tabs(file, tabs);
		// using double pointer iterators as we need the key.
		fprintf(file, "\"%s\": ", (*iter)->Key.CStr);
		
		switch( type ) {
			case HarbolCfgType_Null:
				fputs("null\n", file); break;
			case HarbolCfgType_Linkmap:
				fputs("{\n", file);
				__harbol_cfg_build_file(*(struct HarbolLinkMap **)v->Data, file, tabs+1);
				__write_tabs(file, tabs);
				fputs("}\n", file);
				break;
			
			case HarbolCfgType_String:
				fprintf(file, "\"%s\"\n", (*(struct HarbolString **)v->Data)->CStr); break;
			case HarbolCfgType_Float:
				fprintf(file, "%" PRIfMAX "\n", *(floatmax_t *)v->Data); break;
			case HarbolCfgType_Int:
				fprintf(file, "%" PRIiMAX "\n", *(intmax_t *)v->Data); break;
			case HarbolCfgType_Bool:
				fprintf(file, "%s\n", (*(bool *)v->Data) ? "true" : "false"); break;
			
			case HarbolCfgType_Color: {
				struct { uint8_t r,g,b,a; } *color = (void*)v->Data;
				fprintf(file, "c[ %u, %u, %u, %u ]\n", color->r, color->g, color->b, color->a);
				break;
			}
			case HarbolCfgType_Vec4D: {
				struct { float x,y,z,w; } *vec4 = (void*)v->Data;
				fprintf(file, "v[ %f, %f, %f, %f ]\n", vec4->x, vec4->y, vec4->z, vec4->w);
				break;
			}
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
