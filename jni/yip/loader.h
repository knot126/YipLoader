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

#pragma once

#include "extern/leaf.h"
#include "yiploader.h"

/* Globals */
extern struct android_app *gApp;
extern Leaf *gLeaf;
extern void *gLibAndroid;
extern void *gLibC;
extern char *gGameName;
extern char *gPackageCodePath;
extern YipModInfo *gModChain;

/* Create/destroy YipLoader functions */
const char *YipLoader_EarlyInit(void);
const char *YipLoader_Init(void);
void YipLoader_Release(void);

/* Load game */
const char *YipLoader_LoadGame(void);
const char *YipLoader_LoadMods(void);
