/**
 * The YipLoader API as used by mods
 */

#ifndef _YIP_LOADER_H_
#define _YIP_LOADER_H_

#include <stdint.h>
#include "extern/leaf.h"

/**
 * A generic buffer type
 */
typedef struct YipBuffer {
	size_t size;
	uint8_t *data;
} YipBuffer;

/**
 * Information about a specific mod
 */
typedef struct YipModInfo YipModInfo;

typedef struct YipModInfo {
	/* To be filled by the mod itself */
	const char *name; // e.g. "KnotUtils"
	const char *author; // e.g. "knot126"
	const char *description; // e.g. "Provides extra utilities for in-game scripts"
	const char *game; // e.g. "smashhit" (or "*" for all games)
	uint32_t version; // verison code e.g. 10402 for 1.4.2
	YipModInfo *assumes; // List of partial mod infos that this mod assumes already exists or NULL
	YipModInfo *conflicts; // List of partial mod infos describing mods that this mod conflicts with or NULL
	YipModInfo *next; // Next mod in the chain
	
	/* YipLoader internal (but accessible by public) */
	void *dl_handle;
} YipModInfo;

/**
 * Init, tick and destroy function types
 */
typedef void (*YipModConstructor)(void);

/**
 * Opaque detour object type, which is like a hook, but:
 *   - They are undoable
 *   - The original function cannot be called without undoing the hook
 *   - They support some functions that are normally too small to be hooked
 */
typedef struct YipDetour YipDetour;

// Utilities for mods (symbol lookup, getting current game, etc.)
void *YipLookupSymbol(const char *symbol);

void *YipHookFunction(const char *symbol, void *hook, bool replace);
void *YipHookFunctionAt(size_t vaddr, void *hook, bool replace);

YipDetour *YipDetourFunction(const char *symbol, void *detour);
YipDetour *YipDetourFunctionAt(size_t vaddr, size_t func_size, void *detour);
void       YipDetourSwap(YipDetour *self);

bool  YipReplaceFunction(const char *symbol, void *replacement);
bool  YipReplaceFunctionAt(size_t vaddr, size_t func_size, void *replacement);

bool  YipPatch(size_t vaddr, YipBuffer buffer);
bool  YipPatchv2(size_t vaddr, YipBuffer buffer, YipBuffer *original);

struct android_app;

#define YipDestroyBuffer(BUFFER) (free((BUFFER).data))

const char *YipGetGameName(void);
struct android_app *YipGetAndroidAppStruct(void);
Leaf *YipGetLeafInstance(void);

// Get first mod in the linked list of mods kept by YipLoader
const YipModInfo *YipGetModList(void);

#endif
