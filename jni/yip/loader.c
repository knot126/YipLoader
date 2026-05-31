/**
 * Handle loading the target game (using Leaf), built-in modules and extensions.
 * 
 * -----------------------------------------------------------------------------
 * 
 * This file is part of KnShim. Copyright (c) 2025 - 2026 Knot126.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <android_native_app_glue.h>
#include <android/log.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

#define LEAF_IMPLEMENTATION
#include "extern/leaf.h"
#undef LEAF_IMPLEMENTATION

#include "util.h"
#include "arch.h"
#include "jnistuff.h"
#include "apkiter.h"
#include "loader.h"
#include "yiploader.h"

/* Shim-wide globals. They are kept here since they are used here most. */
struct android_app *gApp;
Leaf *gLeaf;
void *gLibAndroid;
void *gLibC;
char *gGameName;
char *gPackageCodePath;

/* Early init, init, and release */
const char *YipLoader_EarlyInit(void) {
	/**
	 * Initialise some core stuff the shim needs. This happens *before* Smash
	 * Hit is loaded.
	 */
	
	// dynamically load libandroid.so for functions that might not be available
	// in older api levels and thus cannot be statically linked if we want to
	// keep running on these older versions.
	gLibAndroid = dlopen("libandroid.so", RTLD_NOW | RTLD_GLOBAL);
	
	if (!gLibAndroid) {
		return "Loading libandroid.so failed";
	}
	
	// same goes for libc
	gLibC = dlopen("libc.so", RTLD_NOW | RTLD_GLOBAL);
	
	if (!gLibC) {
		return "Loading libc.so failed";
	}
	
	return NULL;
}

const char *YipLoader_Init(void) {
	atexit(&YipLoader_Release);
	return NULL;
}

void YipLoader_Release(void) {
	LeafFree(gLeaf);
}

/* Game loading */
static inline AAsset *YipLoader_LoadMainSharedObject(const char *path, const void **data, size_t *length) {
	AAssetManager *asset_manager = gApp->activity->assetManager;
	
	AAsset *asset = AAssetManager_open(asset_manager, path, AASSET_MODE_BUFFER);
	
	if (!asset) {
		return NULL;
	}
	
	length[0] = AAsset_getLength(asset);
	data[0] = AAsset_getBuffer(asset);
	
	return asset;
}

static inline char *YipLoader_FindGameObject(void) {
	/**
	 * Find any single shared object in the native/<current-arch> path and 
	 * return the path to it. This is a fairly clean way to load the game .so
	 * without needing to know the exact game name ahead of time. The returned
	 * string must be freed!
	 */
	
	const char *const subdir = "native/" KN_ARCH_STRING;
	AAssetDir *natives = AAssetManager_openDir(gApp->activity->assetManager, subdir);
	if (!natives) { return NULL; }
	const char *filename_am = AAssetDir_getNextFileName(natives);
	char *filename = malloc(strlen(subdir) + strlen(filename_am) + 2);
	if (!filename) { goto finally; }
	strcpy(filename, subdir);
	strcat(filename, "/");
	strcat(filename, filename_am);
	
finally:
	AAssetDir_close(natives);
	return filename;
}

static inline char *YipLoader_NameOfGameFromObjectPath(const char *path) {
	/**
	 * Parse out the name of the game from the given object path. Returned
	 * string should be freed (unless you rely on the OS to do that after
	 * closing).
	 */
	
	const char *lib = strstr(path, "lib");
	
	if (!lib) {
		return NULL;
	}
	
	lib += 3; // skip "lib"
	
	const char *end = strstr(path, ".so");
	
	if (!end) {
		return NULL;
	}
	
	return strndup(lib, end - lib);
}

const char *YipLoader_LoadGame(void) {
	/**
	 * Find and load the main game binary
	 */
	
	// Create an instance of Leaf for loading the main binary
	gLeaf = LeafInit();
	
	if (!gLeaf) {
		return "Leaf init failed";
	}
	
	// Find shared object path for this game
	char *so_path = YipLoader_FindGameObject();
	
	if (!so_path) {
		return "Could not find any shared object for the current archiecture";
	}
	
	LogI("Found game object: %s", so_path);
	
	// Find name of game
	gGameName = YipLoader_NameOfGameFromObjectPath(so_path);
	
	if (!gGameName) {
		return "Could not parse game name from object path";
	}
	
	LogI("Found game name: %s", gGameName);
	
	// Load the contents of the game's library
	const void *data;
	size_t length;
	AAsset *asset = YipLoader_LoadMainSharedObject(so_path, &data, &length);
	
	if (!asset) {
		return "Failed to load game shared object from shim native dir";
	}
	
	// Load from the buffer we just read
	const char *error = LeafLoadFromBuffer(gLeaf, (void *) data, length);
	
	if (error) {
		LogF("Leaf loading elf failed: %s", error);
		return "Leaf loading elf failed";
	}
	
	// Close asset handle, its not needed anymore
	AAsset_close(asset);
	free(so_path);
	
	return NULL;
}

/* Handle mods themselves */
YipModInfo *gModChain;

static int YipLoader_ZIPFileNameIterationCallback(void *context, const char *name) {
	// Check if this is a mod
	if (strncmp(name, "lib/" KN_ARCH_STRING "/lib", strlen("lib/" KN_ARCH_STRING "/lib"))) {
		return 1;
	}
	
	char suffix[128];
	snprintf(suffix, 128, ".%s.so", gGameName);
	
	if (strlen(name) < strlen(suffix) || strcmp(name + strlen(name) - strlen(suffix), suffix)) {
		LogI("Excluding possible mod %s: not named like a module (missing '%s')", name, suffix);
		return 1;
	}
	
	name += 4;
	
	// Actually start to load it
	LogI("Will now load %s as a module", name);
	
	void *handle = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
	
	char *error = dlerror();
	
	if (error) {
		LogE("Failed to load module %s: %s. Check that the mod isn't corrupt.", name, error);
		return 1;
	}
	
	YipModInfo *mod_info = dlsym(handle, "mod_info");
	
	error = dlerror();
	
	if (error) {
		LogE("Failed to load module %s: %s. Check that the mod contains a valid 'mod_info' symbol.", name, error);
		dlclose(handle);
		return 1;
	}
	
	mod_info->next = gModChain;
	mod_info->dl_handle = handle;
	gModChain = mod_info;
	
	return 1;
}

static bool YipLoader_ModMatchesCriteria(YipModInfo *mod, YipModInfo *crit) {
	if (crit->version != 0 && mod->version != crit->version) {
		return false;
	}
	
	if (crit->name && strcmp(mod->name, crit->name)) {
		return false;
	}
	
	if (crit->author && strcmp(mod->author, crit->author)) {
		return false;
	}
	
	if (crit->game && strcmp(crit->game, "*") && strcmp(mod->game, crit->game)) {
		return false;
	}
	
	return true;
}

static void YipLoader_ValidateMods(void) {
	/**
	 * TODO: seriously we should probably validate the mods at least A LITTLE
	 */
}

static void YipLoader_InitMods(void) {
	/**
	 * Call mod_init() functions.
	 */
	
	YipModInfo *current = gModChain;
	
	while (current) {
		YipModConstructor init = dlsym(current->dl_handle, "mod_init");
		
		if (init) {
			init();
		}
		
		current = current->next;
	}
}

const char *YipLoader_LoadMods(void) {
	gPackageCodePath = YipLoader_GetPackageCodePath();
	
	if (gPackageCodePath) {
		LogI("Found package code path: %s", gPackageCodePath);
	}
	else {
		return "Could not get package code path";
	}
	
	int error = YipLoader_ForEachZIPFileEntry(gPackageCodePath, NULL, YipLoader_ZIPFileNameIterationCallback);
	
	if (error) {
		LogE("YipLoader_ForEachZIPFileEntry returned %d", error);
		return "Failed to find modules for loading";
	}
	
	YipLoader_ValidateMods();
	YipLoader_InitMods();
	
	return NULL;
}
