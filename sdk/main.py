"""
This file is part of KnShim. Copyright (c) 2025 - 2026 Knot126.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import argparse
import sys
import os
import tomllib
import shutil

from pathlib import Path

SDK_DIR = str(Path(__file__).parent.parent)

def create():
	print("Welcome to the YipLoader SDK project generator.")
	print("Please fill in some information about your project.")
	print()
	game = input("Game name: ")
	name = input("Project name: ")
	author = input("Project author: ")
	desc = input("Description: ")
	print()
	
	print("Generate info...")
	os.makedirs(f"jni/{name}", exist_ok=True)
	
	Path(f"jni/{name}/mod_info.c").write_text(f"""// Automatically generated information about this modification.
#include <yiploader/yiploader.h>

YipModInfo yiploader_version = {{
	.name = "YipLoader",
	.version = 1,
}};

YipModInfo mod_info = {{
	.name = "{name.encode('unicode-escape').decode('utf-8')}",
	.author = "{author.encode('unicode-escape').decode('utf-8')}",
	.description = "{desc.encode('unicode-escape').decode('utf-8')}",
	.game = "{game.encode('unicode-escape').decode('utf-8')}",
	.version = 10000,
	.assumes = &yiploader_version,
	.conflicts = NULL,
}};
""")
	
	Path(f"jni/{name}/main.c").write_text("#include <yiploader/yiploader.h>\n\nconst char *mod_init(void) {\n\t// Code to init your mod goes here!\n\treturn NULL;\n}")
	
	print("Generate makefiles...")
	Path(f"jni/Application.mk").write_text("APP_ABI := arm64-v8a armeabi-v7a\nAPP_PLATFORM := android-19\n")
	Path(f"jni/Android.mk").write_text(f"""LOCAL_PATH := $(call my-dir)

# Setup KnShim related stuff
include $(CLEAR_VARS)
LOCAL_MODULE := yip-prebuilt
LOCAL_SRC_FILES := yip/$(TARGET_ARCH_ABI)/libYipLoader.so
include $(PREBUILT_SHARED_LIBRARY)

# This is the setup for YOUR project!
include $(CLEAR_VARS)

LOCAL_ARM_MODE  := arm

# Your module's name
LOCAL_MODULE    := {name}.{game}

# The source files for your module. Don't remove mod_info.c; it's required!
LOCAL_SRC_FILES := {name}/mod_info.c \\\n\t{name}/main.c

# Link against any extra libraries you might need here
# LOCAL_LDLIBS     := -llog -landroid -lGLESv2

# Link against YipLoader itself
LOCAL_SHARED_LIBRARIES := yip-prebuilt

# Include YipLoader's headers
LOCAL_C_INCLUDES += $(LOCAL_PATH)/yip

# Consider providing C flags
# LOCAL_CFLAGS     := -DDUMMY

include $(BUILD_SHARED_LIBRARY)""")
	
	print("Copy pre-built libraries and headers...")
	os.makedirs("jni/yip/yiploader", exist_ok=True)
	shutil.copytree(f"{SDK_DIR}/jni/yip", f"jni/yip/yiploader", dirs_exist_ok=True)
	shutil.copytree(f"{SDK_DIR}/libs/", f"jni/yip", dirs_exist_ok=True)

def upgrade():
	print("Copy pre-built libraries and headers...")
	shutil.copytree(f"{SDK_DIR}/jni/yip", f"jni/yip/yiploader", dirs_exist_ok=True)
	shutil.copytree(f"{SDK_DIR}/libs/", f"jni/yip", dirs_exist_ok=True)

def build():
	os.system('ndk-build')

def main():
	if len(sys.argv) < 2:
		print("Second argument should be one of: 'create', 'build', 'upgrade'")
		return
	
	match sys.argv[1]:
		case 'create':
			create()
		case 'upgrade':
			upgrade()
		case 'build':
			build()
		case _:
			print(f"Unknown option: {sys.argv[1]}")

if __name__ == "__main__":
	main()
