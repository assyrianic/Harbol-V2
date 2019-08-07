#include "plugins.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#	include <windows.h>
#	include <direct.h>
#else
#	include <dlfcn.h>
#	include <unistd.h>
#endif


#ifdef OS_WINDOWS
#	ifndef MODULE_LOAD
#		define MODULE_LOAD(str)   LoadLibrary(str)
#	endif
#	ifndef MODULE_GET_FUNC
#		define MODULE_GET_FUNC    GetProcAddress
#	endif
#	ifndef MODULE_CLOSE
#		define MODULE_CLOSE       FreeLibrary
#	endif
#else
#	ifndef MODULE_LOAD
#		define MODULE_LOAD(str)   dlopen(str, RTLD_NOW | RTLD_GLOBAL)
#	endif
#	ifndef MODULE_GET_FUNC
#		define MODULE_GET_FUNC    dlsym
#	endif
#	ifndef MODULE_CLOSE
#		define MODULE_CLOSE       dlclose
#	endif
#endif

#ifdef OS_WINDOWS
#	ifndef DIRECTORY_SEP
#		define DIRECTORY_SEP    "\\"
#	endif
#	ifndef LIB_EXT
#		define LIB_EXT          "dll"
#	endif
#elif defined OS_LINUX_UNIX
#	ifndef DIRECTORY_SEP
#		define DIRECTORY_SEP    "/"
#	endif
#	ifndef LIB_EXT
#		define LIB_EXT          "so"
#	endif
#elif defined OS_MAC
#	ifndef DIRECTORY_SEP
#		define DIRECTORY_SEP    "/"
#	endif
#	ifndef LIB_EXT
#		define LIB_EXT          "dylib"
#	endif
#endif


HARBOL_EXPORT struct HarbolPlugin harbol_plugin_create(HarbolDLL module, const char libpath[restrict static 1])
{
	struct HarbolPlugin plugin = { harbol_string_create(libpath), NULL, module };
	return plugin;
}

HARBOL_EXPORT bool harbol_plugin_clear(struct HarbolPlugin *const plugin)
{
	if( plugin->SharedObj != NULL )
		MODULE_CLOSE(plugin->SharedObj), plugin->SharedObj=NULL;
	
	harbol_string_clear(&plugin->LibPath);
	plugin->Name = NULL;
	return true;
}

HARBOL_EXPORT HarbolDLL harbol_plugin_shared_obj(const struct HarbolPlugin *const plugin)
{
	return plugin->SharedObj;
}

HARBOL_EXPORT const char *harbol_plugin_name(const struct HarbolPlugin *const plugin)
{
	return( plugin->Name==NULL ) ? NULL : plugin->Name->CStr;
}

HARBOL_EXPORT const char *harbol_plugin_path(const struct HarbolPlugin *plugin)
{
	return plugin->LibPath.CStr;
}

HARBOL_EXPORT void *harbol_plugin_sym(const struct HarbolPlugin *const restrict plugin, const char sym_name[restrict static 1])
{
	return( plugin->SharedObj==NULL ) ? NULL : (void *)(uintptr_t)MODULE_GET_FUNC(plugin->SharedObj, sym_name);
}

HARBOL_EXPORT bool harbol_plugin_reload(struct HarbolPlugin *const plugin)
{
	if( plugin->LibPath.CStr==NULL )
		return false;
	else {
		if( plugin->SharedObj != NULL )
			MODULE_CLOSE(plugin->SharedObj), plugin->SharedObj=NULL;
		
		plugin->SharedObj = MODULE_LOAD(plugin->LibPath.CStr);
		return plugin->SharedObj != NULL;
	}
}

/************************************************************************************/

HARBOL_EXPORT struct HarbolPluginMod *harbol_plugin_mod_new(const char dir[restrict static 1], void *const restrict userdata, const bool load_plugins, HarbolPluginEvent load_cb)
{
	struct HarbolPluginMod *const restrict mod = calloc(1, sizeof *mod);
	if( mod != NULL )
		*mod = harbol_plugin_mod_create(dir, userdata, load_plugins, load_cb);
	return mod;
}

static NEVER_NULL(1, 2) void __load_plugin(struct HarbolPluginMod *const restrict mod, tinydir_file *const restrict f, HarbolPluginEvent load_cb)
{
	HarbolDLL dll = MODULE_LOAD(f->path);
	if( dll==NULL ) {
#	ifdef OS_WINDOWS
		fprintf(stderr, "Harbol Plugin Manager Error: **** Unable to load dll: '%s' ****\n", f->name);
#	else
		fprintf(stderr, "Harbol Plugin Manager Error: **** %s ****\n", dlerror());
#	endif
	} else {
		struct HarbolPlugin plugin = harbol_plugin_create(dll, f->path);
		char *ext_dot = strchr(f->name, '.');
		if( ext_dot != NULL )
			*ext_dot = 0;
		
		const bool res = harbol_linkmap_insert(&mod->Plugins, f->name, &plugin);
		if( res ) {
			struct HarbolKeyVal *const kv = harbol_linkmap_key_get_kv(&mod->Plugins, f->name);
			struct HarbolPlugin *pl = harbol_linkmap_key_get(&mod->Plugins, f->name);
			pl->Name = &kv->Key;
			if( load_cb != NULL )
				load_cb(mod, pl);
		} else {
			harbol_plugin_clear(&plugin);
		}
	}
}

static NEVER_NULL(1, 2) bool __recursive_scan_plugin_dir(struct HarbolPluginMod *const restrict mod, tinydir_dir *const restrict dir, HarbolPluginEvent load_cb)
{
	while( dir->has_next ) {
		tinydir_file file;
		if( tinydir_readfile(dir, &file)<0 )
			continue;
		else if( file.is_dir ) {
			if( file.name[0]=='.' ) {
				/* jumping to tinydir_next at end of loop so we can advance the dir iterator. */
				goto dir_iter_loop;
			}
			tinydir_dir sub_dir;
			if( tinydir_open(&sub_dir, file.path)<0 ) {
				/* jumping to tinydir_next at end of loop so we can advance the dir iterator. */
				goto dir_iter_loop;
			}
			else __recursive_scan_plugin_dir(mod, &sub_dir, load_cb);
		} else if( !strcmp(file.extension, LIB_EXT) ) {
			__load_plugin(mod, &file, load_cb);
		}
dir_iter_loop:;
		if( tinydir_next(dir)<0 )
			break;
	}
	tinydir_close(dir);
	return mod->Plugins.Vec.Count > 0;
}

static NEVER_NULL(1, 2, 3) bool __recursive_scan_by_name(struct HarbolPluginMod *const restrict mod, tinydir_dir *const restrict dir, const char plugin_name[restrict static 1], HarbolPluginEvent load_cb)
{
	while( dir->has_next ) {
		tinydir_file file;
		if( tinydir_readfile(dir, &file)<0 )
			continue;
		else if( file.is_dir ) {
			if( file.name[0]=='.' ) {
				/* jumping to tinydir_next at end of loop so we can advance the dir iterator. */
				goto dir_iter_loop;
			}
			tinydir_dir sub_dir;
			if( tinydir_open(&sub_dir, file.path)<0 ) {
				/* jumping to tinydir_next at end of loop so we can advance the dir iterator. */
				goto dir_iter_loop;
			}
			else __recursive_scan_by_name(mod, &sub_dir, plugin_name, load_cb);
		} else if( !strcmp(file.extension, LIB_EXT) ) {
			char *ext_dot = strchr(file.name, '.');
			if( ext_dot != NULL )
				*ext_dot = 0;
			if( !strcmp(plugin_name, file.name) )
				__load_plugin(mod, &file, load_cb);
		}
dir_iter_loop:;
		if( tinydir_next(dir)<0 )
			break;
	}
	tinydir_close(dir);
	return mod->Plugins.Vec.Count > 0;
}

HARBOL_EXPORT struct HarbolPluginMod harbol_plugin_mod_create(const char dir[restrict static 1], void *const userdata, const bool load_plugins, HarbolPluginEvent load_cb)
{
	struct HarbolPluginMod mod = { harbol_linkmap_create(sizeof(struct HarbolPlugin)), harbol_string_create(NULL), userdata };
	
	// FILENAME_MAX is defined in <stdio.h>
	char currdir[FILENAME_MAX] = {0};
#ifdef OS_WINDOWS
	if( GetCurrentDir(sizeof currdir, currdir) != 0 )
#else
	if( getcwd(currdir, sizeof currdir) != NULL )
#endif
	{
		harbol_string_format(&mod.Dir, "%s%s%s", currdir, DIRECTORY_SEP, dir);
		if( load_plugins )
			harbol_plugin_mod_load_plugins(&mod, load_cb);
		return mod;
	}
	else return mod;
}

HARBOL_EXPORT bool harbol_plugin_mod_clear(struct HarbolPluginMod *const mod, HarbolPluginEvent unload_cb)
{
	harbol_plugin_mod_unload_plugins(mod, unload_cb);
	harbol_string_clear(&mod->Dir);
	return true;
}

HARBOL_EXPORT bool harbol_plugin_mod_free(struct HarbolPluginMod **const managerref, HarbolPluginEvent unload_cb)
{
	if( *managerref==NULL )
		return false;
	else {
		harbol_plugin_mod_clear(*managerref, unload_cb);
		free(*managerref), *managerref=NULL;
		return true;
	}
}

HARBOL_EXPORT struct HarbolPlugin *harbol_plugin_mod_name_get_plugin(const struct HarbolPluginMod *const restrict mod, const char plugin_name[restrict static 1])
{
	return harbol_linkmap_key_get(&mod->Plugins, plugin_name);
}

HARBOL_EXPORT struct HarbolPlugin *harbol_plugin_mod_index_get_plugin(const struct HarbolPluginMod *const mod, const uindex_t index)
{
	return harbol_linkmap_index_get(&mod->Plugins, index);
}

HARBOL_EXPORT size_t harbol_plugin_mod_plugin_count(const struct HarbolPluginMod *const mod)
{
	return mod->Plugins.Vec.Count;
}

HARBOL_EXPORT const char *harbol_plugin_mod_get_dir(const struct HarbolPluginMod *const mod)
{
	return mod->Dir.CStr;
}

HARBOL_EXPORT void *harbol_plugin_mod_userdata(const struct HarbolPluginMod *const mod)
{
	return mod->UserData;
}


HARBOL_EXPORT bool harbol_plugin_mod_name_load_plugin(struct HarbolPluginMod *const restrict mod, const char plugin_name[restrict static 1], HarbolPluginEvent load_cb)
{
	tinydir_dir dir;
	if( tinydir_open(&dir, mod->Dir.CStr)<0 ) {
		fprintf(stderr, "Harbol Plugin Manager Error: **** Unable to Open Dir: '%s' ****\n", mod->Dir.CStr);
		harbol_string_clear(&mod->Dir);
		tinydir_close(&dir);
		return false;
	}
	else return __recursive_scan_by_name(mod, &dir, plugin_name, load_cb);
}

HARBOL_EXPORT bool harbol_plugin_mod_name_del_plugin(struct HarbolPluginMod *const restrict mod, const char plugin_name[restrict static 1], HarbolPluginEvent unload_cb)
{
	struct HarbolPlugin *plugin = harbol_plugin_mod_name_get_plugin(mod, plugin_name);
	if( plugin==NULL ) {
		return false;
	} else {
		if( unload_cb != NULL )
			unload_cb(mod, plugin);
		harbol_plugin_clear(plugin);
		harbol_linkmap_key_del(&mod->Plugins, plugin_name, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_plugin_mod_index_del_plugin(struct HarbolPluginMod *const mod, const uindex_t index, HarbolPluginEvent unload_cb)
{
	struct HarbolPlugin *plugin = harbol_linkmap_index_get(&mod->Plugins, index);
	if( plugin==NULL ) {
		return false;
	} else {
		if( unload_cb != NULL )
			unload_cb(mod, plugin);
		harbol_plugin_clear(plugin);
		harbol_linkmap_index_del(&mod->Plugins, index, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_plugin_mod_load_plugins(struct HarbolPluginMod *const mod, HarbolPluginEvent load_cb)
{
	tinydir_dir dir;
	if( tinydir_open(&dir, mod->Dir.CStr)<0 ) {
		fprintf(stderr, "Harbol Plugin Manager Error: **** Unable to Open Dir: '%s' ****\n", mod->Dir.CStr);
		harbol_string_clear(&mod->Dir);
		tinydir_close(&dir);
		return false;
	} else {
		return __recursive_scan_plugin_dir(mod, &dir, load_cb);
	}
}

HARBOL_EXPORT bool harbol_plugin_mod_unload_plugins(struct HarbolPluginMod *const mod, HarbolPluginEvent unload_cb)
{
	for( uindex_t i=0; i<mod->Plugins.Vec.Count; i++ ) {
		struct HarbolPlugin *plugin = harbol_linkmap_index_get(&mod->Plugins, i);
		if( unload_cb != NULL )
			unload_cb(mod, plugin);
		harbol_plugin_clear(plugin);
	}
	harbol_linkmap_clear(&mod->Plugins, NULL);
	return true;
}

HARBOL_EXPORT bool harbol_plugin_mod_reload_plugins(struct HarbolPluginMod *const mod, HarbolPluginEvent prereload_cb, HarbolPluginEvent postreload_cb)
{
	for( uindex_t i=0; i<mod->Plugins.Vec.Count; i++ ) {
		struct HarbolPlugin *plugin = harbol_linkmap_index_get(&mod->Plugins, i);
		if( prereload_cb != NULL )
			prereload_cb(mod, plugin);
			
		harbol_plugin_reload(plugin);
		
		if( postreload_cb != NULL )
			postreload_cb(mod, plugin);
	}
	return true;
}
