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
 * Same as the previous struct but readonly and containing only public fields
 */
typedef struct YipModInfoStatic YipModInfoStatic;

typedef struct YipModInfoStatic {
	/* To be filled by the mod itself */
	const char * const name; // e.g. "KnotUtils"
	const char * const author; // e.g. "knot126"
	const char * const description; // e.g. "Provides extra utilities for in-game scripts"
	const char * const game; // e.g. "smashhit" or "*" for all games
	const uint32_t version; // verison code e.g. 10402 for 1.4.2
	YipModInfoStatic * const assumes; // List of partial mod infos that this mod assumes already exists or NULL
	YipModInfoStatic * const conflicts; // List of partial mod infos describing mods that this mod conflicts with or NULL
	YipModInfoStatic * const next; // Next mod in the chain
} YipModInfoStatic;

/**
 * Init, tick and destroy function types
 */
typedef void (*YipModConstructor)(void);

// Utilities for mods (symbol lookup, getting current game, etc.)
void *YipLookupSymbol(const char *symbol);
void *YipHookFunction(const char *symbol, void *hook, bool replace);
void *YipHookFunctionAt(size_t vaddr, void *hook, bool replace);
bool YipPatch(size_t vaddr, YipBuffer buffer);
bool YipPatchv2(size_t vaddr, YipBuffer buffer, YipBuffer *original);

struct android_app;

#define YipDestroyBuffer(BUFFER) (free((BUFFER).data))

const char *YipGetGameName(void);
struct android_app *YipGetAndroidAppStruct(void);
Leaf *YipGetLeafInstance(void);

// Get first mod in the linked list of mods kept by YipLoader
const YipModInfoStatic *YipGetModList(void);

#endif
