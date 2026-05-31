#!/usr/bin/env python3
import os
import shutil
import sys

status = os.system(f"ndk-build")

if not status:
	if "--upgrade" in sys.argv:
		apks = os.listdir("/tmp/apk-editor-studio/apk")
		
		if len(apks) > 0:
			apk_path = f"/tmp/apk-editor-studio/apk/{apks[0]}"
			print(f"Upgrade apk at {apk_path}")
			shutil.copytree("./libs", f"{apk_path}/lib", dirs_exist_ok=True)
		else:
			print(f"No APKs to upgrade")
	
	if "--package" in sys.argv:
		version = sys.argv[sys.argv.index("--package")+1]
		shutil.make_archive(f"knshim-r{version}-{game}-libs", "zip", "./libs")
