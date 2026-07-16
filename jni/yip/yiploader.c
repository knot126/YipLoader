/**
 * Implementations of YipLoader features provided for mods to use
 * 
 * -----------------------------------------------------------------------------
 * 
 * This file is part of KnShim. Copyright (c) 2024 - 2026 Knot126.
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

#include "loader.h"
#include "util.h"

#include "yiploader.h"

/// @section leafhook_setup
#if defined(__ARM_ARCH_7A__)
#define LH_AARCH32
#define LH_HOOK_SIZE 12
#elif defined(__aarch64__)
#define LH_AARCH64
#define LH_HOOK_SIZE 16
#else
#define LH_HOOK_SIZE (-1)
#endif

#define LEAFHOOK_IMPLEMENTATION
#include "extern/leafhook.h"
/// end of that

void *YipLookupSymbol(const char *symbol) {
	/**
	 * Get the address of a symbol in the main game binary.
	 */
	
	return LeafSymbolAddr(gLeaf, symbol);
}

LHHooker *gHookingContext;

static bool YipLoader_HookInit(void) {
	gHookingContext = LHHookerCreate();
	
	return !!gHookingContext;
}

#define ENSURE_HOOKING_CONTEXT() if (!gHookingContext) {\
		if (!YipLoader_HookInit()) {\
			LogE("Could not start LeafHook library");\
			return NULL;\
		}\
	}

void *YipHookFunction(const char *symbol, void *hook, bool replace) {
	/**
	 * Hook a function given the symbol name, the hook to use, and weather or
	 * not to replace the function entirely or to return a pointer to the
	 * original.
	 * 
	 * If replace is false, then the original function pointer is returned on
	 * success. If replace is true, then the address of the symbol is returned
	 * on success. If the function fails, it always returns NULL.
	 */
	
	ENSURE_HOOKING_CONTEXT();
	
	LeafSym *sym_info = LeafSymbolInfo(gLeaf, symbol);
	
	// We can't hook small functions for now.
	if (sym_info->st_size < LH_HOOK_SIZE) {
		return NULL;
	}
	
	void *func = (void *) sym_info->st_value;
	void *orig = func;
	
	if (LHHookerHookFunction(gHookingContext, func, hook, replace ? NULL : &orig)) {
		return orig;
	}
	
	return NULL;
}

void *YipHookFunctionAt(size_t vaddr, void *hook, bool replace) {
	/**
	 * Hook a function given its base address, the hook to use, and weather or
	 * not to replace the function entirely or to return a pointer to the
	 * original.
	 * 
	 * In almost all cases, you should be using YipHookFunction as it's far more
	 * version agonostic.
	 * 
	 * If replace is false, then the original function pointer is returned on
	 * success. If replace is true, then the address of the symbol is returned
	 * on success. If the function fails, it always returns NULL.
	 */
	
	ENSURE_HOOKING_CONTEXT();
	
	void *func = LeafGetRealAddr(gLeaf, vaddr);
	void *orig = func;
	
	if (LHHookerHookFunction(gHookingContext, func, hook, replace ? NULL : &orig)) {
		return orig;
	}
	
	return NULL;
}

bool YipPatch(size_t vaddr, YipBuffer buffer) {
	/**
	 * Patch the bytes starting at the virtual address vaddr by replacing them
	 * with the bytes from the given `buffer`.
	 */
	
	return YipPatchv2(vaddr, buffer, NULL);
}

bool YipPatchv2(size_t vaddr, YipBuffer buffer, YipBuffer *original) {
	/**
	 * Patch the bytes starting at the virtual address vaddr by replacing them
	 * with the bytes from the given `buffer`. If original is not NULL, then
	 * a copy of the original bytes is put into a malloc()'d buffer (which must
	 * be freed by the user).
	 */
	
	char *addr = LeafGetRealAddr(gLeaf, vaddr);
	
	if (!addr) {
		return false;
	}
	
	if (original) {
		original->size = buffer.size;
		original->data = malloc(buffer.size);
		
		if (!original->data) {
			return false;
		}
		
		memcpy(original->data, addr, buffer.size);
	}
	
	memcpy(addr, buffer.data, buffer.size);
	
	return true;
}

const char *YipGetGameName(void) {
	/**
	 * Get the name of the currently loaded game as a string.
	 */
	
	return gGameName;
}

struct android_app *YipGetAndroidAppStruct(void) {
	/**
	 * Get the android_app struct passed to android_main() on launch
	 */
	
	return gApp;
}

Leaf *YipGetLeafInstance(void) {
	/**
	 * Get the instance of Leaf used to load the main game
	 */
	
	return gLeaf;
}

const YipModInfoStatic *YipGetModList(void) {
	/**
	 * Get the first node in a linked-list of loaded mods
	 */
	
	return (YipModInfoStatic *) gModChain;
}
