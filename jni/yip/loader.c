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
YipLoader_LinearAllocator gPreSegmentAllocator;
YipLoader_LinearAllocator gPostSegmentAllocator;

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

// Because the main use of the pre and post segments is for trampolines, which
// are generally shorter on 32-bit arches, we specify less pre and post space
// for them than 64-bit arches.
#if defined(__arm__) || defined(__i386__)
#define YIP_PRE_SPACE (32 * 1024) // 32 KiB
#define YIP_POST_SPACE (16 * 1024) // 16 KiB
#else
#define YIP_PRE_SPACE (64 * 1024) // 64 KiB
#define YIP_POST_SPACE (16 * 1024) // 16 KiB
#endif

const char *YipLoader_LoadGame(void) {
	/**
	 * Find and load the main game binary
	 */
	
	// Create an instance of Leaf for loading the main binary
	LeafParams params = {
		.pre_extra_size = YIP_PRE_SPACE,
		.post_extra_size = YIP_POST_SPACE,
	};
	
	gLeaf = LeafInit(&params);
	
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

const char *YipLoader_PostLoadGame(void) {
	// Setup linear allocators for pre and post segments
	size_t pre_size;
	void *pre = LeafGetSegment(gLeaf, LEAF_EXTRA_PRE, &pre_size);
	YipLoader_LinearAllocator_Init(&gPreSegmentAllocator, pre, pre_size, true);
	
	size_t post_size;
	void *post = LeafGetSegment(gLeaf, LEAF_EXTRA_POST, &post_size);
	YipLoader_LinearAllocator_Init(&gPostSegmentAllocator, post, post_size, false);
	
	return NULL;
}

/* Handle mods themselves */
YipModInfo *gModChain;

/* YipLoader's own mod info structure */
YipModInfo yiploader_mod_info = {
	.name = "YipLoader",
	.author = "knot126",
	.description = "Mod loader for Android games",
	.game = NULL,
	.version = 1,
};

typedef struct YipModIterationContext {
	bool hadError;
} YipModIterationContext;

static int YipLoader_ZIPFileNameIterationCallback(YipModIterationContext *context, const char *name) {
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
	
	// skip the "lib/" KN_ARCH_STRING "/" bit
	name += 4 + strlen(KN_ARCH_STRING) + 1;
	
	// Actually start to load it
	LogI("Will now load %s as a module", name);
	
	void *handle = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
	
	char *error = NULL;
	
	if (!handle) {
		error = dlerror();
		
		if (error) {
			LogE("Failed to load module %s: %s. Check that the mod isn't corrupt.", name, error);
			context->hadError = true;
			return 1;
		}
	}
	
	YipModInfo *mod_info = dlsym(handle, "mod_info");
	
	// While returning NULL doesn't technically mean there was an error, we
	// expect it to be non-NULL in our case anyway.
	if (!mod_info) {
		error = dlerror();
		
		if (error) {
			LogE("Failed to load module %s: %s. Check that the mod contains a valid 'mod_info' symbol.", name, error);
			dlclose(handle);
			context->hadError = true;
			return 1;
		}
	}
	else {
		LogI("Loaded mod %s by %s version %d", mod_info->name, mod_info->author, mod_info->version);
	}
	
	mod_info->next = gModChain;
	mod_info->dl_handle = handle;
	gModChain = mod_info;
	
	return 1;
}

static bool YipLoader_ModMatchesCriteria(YipModInfo *mod, YipModInfo *crit) {
	if (mod == crit) {
		return true;
	}
	
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

static bool YipLoader_ValidateMod(YipModInfo *mod_info) {
	bool valid = true;
	
	// Check dependencies
	YipModInfo *current = mod_info->assumes;
	
	while (current) {
		// Check against YipLoader itself
		if (YipLoader_ModMatchesCriteria(&yiploader_mod_info, current)) {
			goto valid;
		}
		
		// Find it in mod chain
		YipModInfo *candidate = gModChain;
		
		while (candidate) {
			if (YipLoader_ModMatchesCriteria(candidate, current)) {
				goto valid;
			}
		}
		
		LogE("Dependency validation for mod %s failed: depends on %s version %d, but mod was not found!", mod_info->name, current->name, current->version);
		
		valid = false;
		
	valid:
		current = current->next;
	}
	
	// Check for conflicting mods
	current = mod_info->conflicts;
	
	while (current) {
		YipModInfo *candidate = gModChain;
		
		while (candidate) {
			if (YipLoader_ModMatchesCriteria(candidate, current)) {
				LogE("Conflict validation for mod %s failed: conflicts with %s version %d", mod_info->name, candidate->name, candidate->version);
				valid = false;
				continue;
			}
			
			candidate = candidate->next;
		}
		
		current = current->next;
	}
	
	return valid;
}

static bool YipLoader_ValidateMods(void) {
	/**
	 * TODO: seriously we should probably validate the mods at least A LITTLE
	 */
	
	bool valid = true;
	
	YipModInfo *mod = gModChain;
	
	while (mod) {
		if (!YipLoader_ValidateMod(mod)) {
			valid = false;
		}
		
		mod = mod->next;
	}
	
	return valid;
}

static void YipLoader_MoveBehind(YipModInfo *mod_info, YipModInfo *crit) {
	/**
	 * Ensure the mod matching mod_info is behind the one matching crit
	 */
	
	YipModInfo *current = gModChain;
	
	YipModInfo *old_prev = NULL;
	YipModInfo *old_current = NULL;
	YipModInfo *new_prev = NULL;
	
	while (current) {
		if (YipLoader_ModMatchesCriteria(current, crit)) {
			if (old_prev && old_current) {
				// Remove from its current position
				old_prev->next = old_current->next;
				
				// Insert at new position, after the current (our dep)
				old_current->next = current->next;
				current->next = old_current;
			}
			else {
				// Otherwise, we're already behind this mod, so we have nothing
				// to do.
			}
			
			return;
		}
		else if (YipLoader_ModMatchesCriteria(current, mod_info)) {
			// If we encouter this before exit, we're ahead and need to move
			old_prev = new_prev;
			old_current = old_current;
		}
		
		new_prev = current;
		current = current->next;
	}
}

static bool YipLoader_InitMods(void) {
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
	
	return true;
}

const char *YipLoader_LoadMods(void) {
	YipModIterationContext zip_iteration_context = {};
	
	gPackageCodePath = YipLoader_GetPackageCodePath();
	
	if (gPackageCodePath) {
		LogI("Found package code path: %s", gPackageCodePath);
	}
	else {
		return "Could not get package code path";
	}
	
	int error = YipLoader_ForEachZIPFileEntry(gPackageCodePath, &zip_iteration_context, (void *) YipLoader_ZIPFileNameIterationCallback);
	
	if (error) {
		LogE("YipLoader_ForEachZIPFileEntry returned %d", error);
		return "Failed to find modules for loading";
	}
	
	if (zip_iteration_context.hadError) {
		return "Errors occured while loading some mods (see log for details)";
	}
	
	if (!YipLoader_ValidateMods()) {
		return "Could not validate mods";
	}
	
	if (!YipLoader_InitMods()) {
		return "Could not init mods";
	}
	
	return NULL;
}
