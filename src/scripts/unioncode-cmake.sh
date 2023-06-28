#!/bin/bash
# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later


CURRENT_DIR="$(pwd)"
SOURCE_DIR=""
BUILD_PROVIDER="cmake"
BUILD_DIR="build"

touch build.log
echo "[INFO] Run CMake Tool in \"${CURRENT_DIR}\" " > build.log
# run generate build configuration in source tree
create_cmake_cache()
{
	if [ -d ${BUILD_DIR} ]; then
		echo "[INFO] Remove Old build cache..." >> build.log
		rm -rf ${BUILD_DIR} >> build.log
	fi
	echo "[INFO] Load CMake to Create CMake Cache..." >> build.log
	cmake -B ${BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 | tee -a build.log	
}

# build target in  build directory
cmake_build_task()
{
	echo "[INFO] Locd CMake to build project" >> build.log
	cmake --build ${BUILD_DIR} | tee -a build.log
}

# build target in build with pipe
load_cmake_build_task()
{
	# check our project was cmake drived
	if [ -e CMakeLists.txt ]; then
		echo "[INFO] Build CMake Based Projects Task Start..." >> build.log
		cmake -B ${BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 | tee -a build.log  && \
			cmake --build ${BUILD_DIR} | tee -a build.log	
		ln -s ${BUILD_DIR}/compile_commands.json compile_commands.json
 	else
		echo "[ERORR] Those Projects Not are CMake Based Project!!!" > build.log
		echo "[ERROR] Please Check YOUR PROJECT Can Build With Other Build System" >> build.log
		exit 1
	fi
	echo "[INFO] Finished Run CMake Tool in \"${CURRENT_DIR}\" " >> build.log
}

# main entry
load_cmake_build_task

exit 0
