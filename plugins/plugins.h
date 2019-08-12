#ifndef HARBOL_PLUGINS_INCLUDED
#	define HARBOL_PLUGINS_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"
#include "../linkmap/linkmap.h"
#include "../tinydir.h"


#ifdef OS_WINDOWS
	typedef HMODULE HarbolDLL;
#else
	typedef void *HarbolDLL;
#endif

struct HarbolPlugin {
	struct HarbolString path, *name;
	HarbolDLL dll;
};

HARBOL_EXPORT NO_NULL struct HarbolPlugin harbol_plugin_create(HarbolDLL module, const char libpath[]);
HARBOL_EXPORT NO_NULL bool harbol_plugin_clear(struct HarbolPlugin *plugin);

HARBOL_EXPORT NO_NULL HarbolDLL harbol_plugin_shared_obj(const struct HarbolPlugin *plugin);
HARBOL_EXPORT NO_NULL const char *harbol_plugin_name(const struct HarbolPlugin *plugin);
HARBOL_EXPORT NO_NULL const char *harbol_plugin_path(const struct HarbolPlugin *plugin);
HARBOL_EXPORT NO_NULL void *harbol_plugin_sym(const struct HarbolPlugin *plugin, const char sym_name[]);

HARBOL_EXPORT NO_NULL bool harbol_plugin_reload(struct HarbolPlugin *plugin);


struct HarbolPluginMod {
	struct HarbolLinkMap plugins;
	struct HarbolString dir;
	void *userdata;
};

/* When an individual plugin has been changed such as being loaded, reloading, or unloaded, this callback will fire.
 * You can use this callback to then individually do whatever you need to the plugin.
 */
typedef void HarbolPluginEvent(struct HarbolPluginMod *mod, struct HarbolPlugin *plugin);

HARBOL_EXPORT NEVER_NULL(1) struct HarbolPluginMod *harbol_plugin_mod_new(const char dir[], void *userdata, bool load_plugins, HarbolPluginEvent load_cb);
HARBOL_EXPORT NEVER_NULL(1) struct HarbolPluginMod harbol_plugin_mod_create(const char dir[], void *userdata, bool load_plugins, HarbolPluginEvent load_cb);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_plugin_mod_clear(struct HarbolPluginMod *mod, HarbolPluginEvent unload_cb);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_plugin_mod_free(struct HarbolPluginMod **modref, HarbolPluginEvent unload_cb);

HARBOL_EXPORT NO_NULL struct HarbolPlugin *harbol_plugin_mod_name_get_plugin(const struct HarbolPluginMod *mod, const char name[]);
HARBOL_EXPORT NO_NULL struct HarbolPlugin *harbol_plugin_mod_index_get_plugin(const struct HarbolPluginMod *mod, uindex_t index);

HARBOL_EXPORT NO_NULL size_t harbol_plugin_mod_plugin_count(const struct HarbolPluginMod *mod);
HARBOL_EXPORT NO_NULL const char *harbol_plugin_mod_get_dir(const struct HarbolPluginMod *mod);
HARBOL_EXPORT NO_NULL void *harbol_plugin_mod_userdata(const struct HarbolPluginMod *mod);

HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_plugin_mod_name_load_plugin(struct HarbolPluginMod *mod, const char name[], HarbolPluginEvent load_cb);
HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_plugin_mod_name_del_plugin(struct HarbolPluginMod *mod, const char name[], HarbolPluginEvent unload_cb);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_plugin_mod_index_del_plugin(struct HarbolPluginMod *mod, uindex_t index, HarbolPluginEvent unload_cb);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_plugin_mod_load_plugins(struct HarbolPluginMod *mod, HarbolPluginEvent load_cb);

HARBOL_EXPORT NEVER_NULL(1) bool harbol_plugin_mod_unload_plugins(struct HarbolPluginMod *mod, HarbolPluginEvent unload_cb);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_plugin_mod_reload_plugins(struct HarbolPluginMod *mod, HarbolPluginEvent prereload_cb, HarbolPluginEvent postreload_cb);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_PLUGINS_INCLUDED */
