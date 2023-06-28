#!/bin/bash
# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later


CURRENT_DIR="$(pwd)"
#SOURCE_DIR=""
BUILD_PROVIDER="ninja"
BUILD_DIR="build"

touch build.log
echo "[INFO] Run Ninja Build Tool in \"${CURRENT_DIR}\" " > build.log
# run generate build configuration in source tree
create_buildninja_with_cmake()
{
	if [ -d ${BUILD_DIR} ]; then
		echo "[INFO] Remove Old build cache..." >> build.log
		rm -rf ${BUILD_DIR} >> build.log
	fi
	echo "[INFO] Load CMake to Create Ninja build.ninja ..." >> build.log
	cmake -GNinja -B ${BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 | tee -a build.log	
}


# build target in build with pipe
load_ninja_build_task()
{
	# check our project was ninja drived
	if [ -e build.ninja ]; then
		# project root directory exist file build.ninja
		echo "[INFO] Build Provider is: ${BUILD_PROVIDER}" >> build.log
		echo "[INFO] Build Ninja Build Based Projects Task Start..." >> build.log
		ninja -C build | tee -a build.log
 	else
		# try to find CMakeLists.txt to create build.ninja in build directory
		echo "[INFO] Build Ninja Build Based Projects Task Start..." >> build.log
		if [ -e CMakeLists.txt ]; then
			echo "[INFO] Build Ninja Use CMake as Meta Build Tool" >> build.log
			create_buildninja_with_cmake
			ninja -C "${BUILD_DIR}" | tee -a build.log
		else
			echo "[ERORR] Those Projects Not are Ninja Build Based Project!!!" > build.log
			echo "[ERROR] Please Check YOUR PROJECT Can Build With Other Build System" >> build.log
			exit 1
		fi
	fi
	echo "[INFO] Finished Run Ninja Build Tool in \"${CURRENT_DIR}\" " >> build.log
}

# main entry
load_ninja_build_task

exit 0
