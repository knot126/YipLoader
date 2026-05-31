/**
 * Various utility functions that don't belong elsewhere.
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

#include <dlfcn.h>

#include "util.h"

extern void *gLibAndroid;

#define LOAD_LIBANDROID_FUNC(RET, NAME, SIG) RET (*NAME)SIG = dlsym(gLibAndroid, #NAME);

int YipLoader_GetDeviceSDK(void) {
	/**
	 * Get the SDK level of the device this app is running on. If less than 24,
	 * this returns -1.
	 */
	
	LOAD_LIBANDROID_FUNC(int, android_get_device_api_level, (void));
	
	if (android_get_device_api_level) {
		return android_get_device_api_level();
	}
	else {
		return -1;
	}
}

int YipLoader_GetAppSDK(void) {
	/**
	 * Get the target SDK of the currently running app. This may return -1 if
	 * the system is running target SDK less than 24.
	 */
	
	LOAD_LIBANDROID_FUNC(int, android_get_application_target_sdk_version, (void));
	
	if (android_get_application_target_sdk_version) {
		return android_get_application_target_sdk_version();
	}
	else {
		return -1;
	}
}
