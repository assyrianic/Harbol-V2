#ifndef HARBOL_CFG_INCLUDED
#	define HARBOL_CFG_INCLUDED

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"
#include "../linkmap/linkmap.h"
#include "../variant/variant.h"

#ifdef __cplusplus
extern "C" {
#endif

enum HarbolCfgType {
	HarbolCfgType_Null,
	HarbolCfgType_Linkmap, // harbol linkmap type
	HarbolCfgType_String, // harbol string type
	HarbolCfgType_Float, // floatmax_t, as defined by Harbol.
	HarbolCfgType_Int, // intmax_t aka long long int
	HarbolCfgType_Bool, // bool
	HarbolCfgType_Color, // 4 byte color array
	HarbolCfgType_Vec4D, // 16 byte float32 array
};

union HarbolColor {
	uint32_t Int;
	struct{ uint8_t R,G,B,A; } Bytes;
};

struct HarbolVec4D {
	float X,Y,Z,W;
};


HARBOL_EXPORT NO_NULL struct HarbolLinkMap *harbol_cfg_parse_file(const char filename[]);
HARBOL_EXPORT NO_NULL struct HarbolLinkMap *harbol_cfg_parse_cstr(const char cstr[]);
HARBOL_EXPORT NO_NULL bool harbol_cfg_free(struct HarbolLinkMap **cfgref);
HARBOL_EXPORT NO_NULL struct HarbolString harbol_cfg_to_str(const struct HarbolLinkMap *cfg);

HARBOL_EXPORT NO_NULL struct HarbolLinkMap *harbol_cfg_get_section(struct HarbolLinkMap *cfg, const char keypath[]);
HARBOL_EXPORT NO_NULL char *harbol_cfg_get_cstr(struct HarbolLinkMap *cfg, const char keypath[]);
HARBOL_EXPORT NO_NULL struct HarbolString *harbol_cfg_get_str(struct HarbolLinkMap *cfg, const char keypath[]);
HARBOL_EXPORT NO_NULL floatmax_t *harbol_cfg_get_float(struct HarbolLinkMap *cfg, const char keypath[]);
HARBOL_EXPORT NO_NULL intmax_t *harbol_cfg_get_int(struct HarbolLinkMap *cfg, const char keypath[]);
HARBOL_EXPORT NO_NULL bool *harbol_cfg_get_bool(struct HarbolLinkMap *cfg, const char keypath[]);
HARBOL_EXPORT NO_NULL union HarbolColor *harbol_cfg_get_color(struct HarbolLinkMap *cfg, const char keypath[]);
HARBOL_EXPORT NO_NULL struct HarbolVec4D *harbol_cfg_get_vec4D(struct HarbolLinkMap *cfg, const char keypath[]);
HARBOL_EXPORT NO_NULL enum HarbolCfgType harbol_cfg_get_type(struct HarbolLinkMap *cfg, const char keypath[]);

HARBOL_EXPORT NO_NULL bool harbol_cfg_set_str(struct HarbolLinkMap *cfg, const char keypath[], const char cstr[], bool override_convert);
HARBOL_EXPORT NO_NULL bool harbol_cfg_set_float(struct HarbolLinkMap *cfg, const char keypath[], floatmax_t fltval, bool override_convert);
HARBOL_EXPORT NO_NULL bool harbol_cfg_set_int(struct HarbolLinkMap *cfg, const char keypath[], intmax_t ival, bool override_convert);
HARBOL_EXPORT NO_NULL bool harbol_cfg_set_bool(struct HarbolLinkMap *cfg, const char keypath[], bool boolval, bool override_convert);
HARBOL_EXPORT NO_NULL bool harbol_cfg_set_color(struct HarbolLinkMap *cfg, const char keypath[], union HarbolColor color, bool override_convert);
HARBOL_EXPORT NO_NULL bool harbol_cfg_set_vec4D(struct HarbolLinkMap *cfg, const char keypath[], struct HarbolVec4D vec4d, bool override_convert);
HARBOL_EXPORT NO_NULL bool harbol_cfg_set_to_null(struct HarbolLinkMap *cfg, const char keypath[]);

HARBOL_EXPORT NO_NULL bool harbol_cfg_build_file(const struct HarbolLinkMap *cfg, const char filename[], bool overwrite);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_CFG_INCLUDED */
