#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
export TOOLCHAINS=$1

echo "[" > $TOOLCHAINS

search_path="/usr/bin"
prefix="x86_64-linux-gnu- aarch64-linux-gnu- mips64el-linux-gnuabi64-"
##################################################################################

# probe c compilers 
probe_c_compilers()
{
	C_COMPILERS=$(find $search_path -name 'gcc' \
		-o -name 'gcc-[1-9]*' \
		-o -name 'x86_64-linux-gnu-gcc' \
		-o -name 'x86_64-linux-gnu-gcc-[1-9]*' \
		-o -name 'aarch64-linux-gnu-gcc' \
		-o -name 'aarch64-linux-gnu-gcc-[1-9]*' \
		-o -name 'mips64el-linux-gnuabi64-gcc' \
		-o -name 'mips64el-linux-gnuabi64-gcc-[1-9]*' \
		-o -name 'clang-[1-9]*' 
		)

	echo "{\"C compilers\": [" >> $TOOLCHAINS
	for compiler in ${C_COMPILERS[@]} clang; do
		echo "{" >> $TOOLCHAINS
		name=$(basename $compiler)
		if [ $name = "clang" ]; then
			path=$(which clang)
			echo "\"name\":\"$name\"," >> $TOOLCHAINS
			echo "\"path\":\"$path\"" >> $TOOLCHAINS
			echo "}" >> $TOOLCHAINS
		else
			echo "\"name\":\"$name\"," >> $TOOLCHAINS
		    echo "\"path\":\"$compiler\"" >> $TOOLCHAINS
			echo "}," >>  $TOOLCHAINS
		fi
	done
	echo "]}" >> $TOOLCHAINS
}

# probe c++ compilers
CXX_COMPILERS="g++ x86_64-linux-gnu-g++ aarch64-linux-gnu-g++ mips64el-linux-gnuabi64-g++ g++-8 x86_64-linux-gnu-g++-8 aarch64-linux-gnu-g++-8 mips64el-linux-gnuabi64-g++-8 clang++"
probe_cxx_compilers()
{
		CXX_COMPILERS=$(find $search_path -name 'g++' \
		-o -name 'gcc-[1-9]*' \
		-o -name 'x86_64-linux-gnu-g++' \
		-o -name 'x86_64-linux-gnu-g++-[1-9]*' \
		-o -name 'aarch64-linux-gnu-g++' \
		-o -name 'aarch64-linux-gnu-g++-[1-9]*' \
		-o -name 'mips64el-linux-gnuabi64-g++' \
		-o -name 'mips64el-linux-gnuabi64-g++-[1-9]*' \
		-o -name 'clang++-[1-9]*' 
		)

	echo "{\"C++ compilers\": [" >> $TOOLCHAINS
	for compiler in ${CXX_COMPILERS[@]} clang++; do
		echo "{" >> $TOOLCHAINS
		name=$(basename $compiler)
		if [ $compiler = clang++ ]; then
			path=$(which clang++)
			echo "\"name\":\"clang++\"," >> $TOOLCHAINS
			echo "\"path\":\"$path\"" >> $TOOLCHAINS
			echo "}" >> $TOOLCHAINS
		else
			echo "\"name\":\"$name\"," >> $TOOLCHAINS
		    echo "\"path\":\"$compiler\"" >> $TOOLCHAINS
			echo "}," >>  $TOOLCHAINS
		fi
	done
	echo "]}" >> $TOOLCHAINS
}

# probe gdb/lldb

probe_debuggers()
{
	DEBUGGERS=($(find /usr/bin -name gdb \
			-o -name lldb-[1-9]* -o -name lldb
		))
	echo "{\"C/C++ debuggers\": [" >> $TOOLCHAINS
	count=${#DEBUGGERS[@]}
	for ((i=0;i<$count;i++)) do
		debugger=${DEBUGGERS[i]}
		echo "{" >> $TOOLCHAINS
		name=$(basename $debugger)
		echo "\"name\":\"$name\"," >> $TOOLCHAINS
		echo "\"path\":\"$debugger\"" >> $TOOLCHAINS			
		echo "}" >>  $TOOLCHAINS
		last=$[$count-1]
		if [ $i -ne $last ]; then
			echo "," >>  $TOOLCHAINS
		fi	
	done
	echo "]}" >> $TOOLCHAINS
}

# probe cmake
probe_build_systems()
{
	CMAKE_VERSIONS=($(find $search_path -name 'cmake'))
	echo "{\"C/C++ build systems\": [" >> $TOOLCHAINS
	count=${#CMAKE_VERSIONS[@]}
	for ((i=0;i<$count;i++)) do
		version=${CMAKE_VERSIONS[i]}
		echo "{" >> $TOOLCHAINS
		name=$(basename $version)
		path=$version
		echo "\"name\":\"$name\"," >> $TOOLCHAINS
		echo "\"path\":\"$path\"" >> $TOOLCHAINS
		echo "}" >>  $TOOLCHAINS
		last=$[$count-1]
		if [ $i -ne $last ]; then
			echo "," >>  $TOOLCHAINS
		fi		
	done
	echo "]}" >> $TOOLCHAINS
}

# probe ninja
probe_ninja()
{
	NINJA_VERSIONS=($(find $search_path -name 'ninja'))
	echo "{\"Ninja\": [" >> $TOOLCHAINS
	count=${#NINJA_VERSIONS[@]}
	for ((i=0;i<$count;i++)) do
		version=${NINJA_VERSIONS[i]}
		echo "{" >> $TOOLCHAINS
		name=$(basename $version)
		path=$version
		echo "\"name\":\"$name\"," >> $TOOLCHAINS
		echo "\"path\":\"$path\"" >> $TOOLCHAINS
		echo "}" >>  $TOOLCHAINS
		last=$[$count-1]
		if [ $i -ne $last ]; then
			echo "," >>  $TOOLCHAINS
		fi		
	done
	echo "]}" >> $TOOLCHAINS
}

# probe clangd
probe_lsp_servers()
{
	LSP_SERVERS=$(find $search_path -name clangd-[1-9]*)
	echo "{\"C/C++ LSP Servers\": [" >> $TOOLCHAINS
	for lsp in ${LSP_SERVERS[@]} clangd; do
		echo "{" >> $TOOLCHAINS
		name=$(basename $lsp)
		if [ $lsp = clangd ]; then
			path=$(which clangd)
			echo "\"name\":\"clangd\"," >> $TOOLCHAINS
	    	echo "\"path\":\"$path\"" >> $TOOLCHAINS
			echo "}" >> $TOOLCHAINS
		else
			echo "\"name\":\"$name\"," >> $TOOLCHAINS
			echo "\"path\":\"$lsp\"" >> $TOOLCHAINS
			echo "}," >>  $TOOLCHAINS
		fi
	done
	echo "]}" >> $TOOLCHAINS
}

# probe jdk
probe_jdk()
{
	JDK_VERSIONS=($(find $search_path -name 'java'))
	echo "{\"JDK\": [" >> $TOOLCHAINS
	count=${#JDK_VERSIONS[@]}
	for ((i=0;i<$count;i++)) do
		jdkversion=${JDK_VERSIONS[i]}
		echo "{" >> $TOOLCHAINS
		name=$(java -version 2>&1 |awk 'NR==1{gsub(/"/,"");print $1 " " $3}')
		path=$jdkversion
		echo "\"name\":\"$name\"," >> $TOOLCHAINS
		echo "\"path\":\"$path\"" >> $TOOLCHAINS
		echo "}" >>  $TOOLCHAINS
		last=$[$count-1]
		if [ $i -ne $last ]; then
			echo "," >>  $TOOLCHAINS
		fi		
	done
	echo "]}" >> $TOOLCHAINS
}

# probe maven
probe_maven()
{
	MAVEN_VERSIONS=($(find $search_path -name 'mvn'))
	echo "{\"Maven\": [" >> $TOOLCHAINS
	count=${#MAVEN_VERSIONS[@]}
	for ((i=0;i<$count;i++)) do
		mavenversion=${MAVEN_VERSIONS[i]}
		echo "{" >> $TOOLCHAINS
		name=$(basename $mavenversion)
		path=$mavenversion
		echo "\"name\":\"$name\"," >> $TOOLCHAINS
		echo "\"path\":\"$path\"" >> $TOOLCHAINS
		echo "}" >>  $TOOLCHAINS
		last=$[$count-1]
		if [ $i -ne $last ]; then
			echo "," >>  $TOOLCHAINS
		fi		
	done
	echo "]}" >> $TOOLCHAINS
}

# probe gradle
probe_gradle()
{
	GRADLE_VERSIONS=($(find $search_path -name 'gradle'))
	echo "{\"Gradle\": [" >> $TOOLCHAINS
	count=${#GRADLE_VERSIONS[@]}
	for ((i=0;i<$count;i++)) do
		gradleversion=${GRADLE_VERSIONS[i]}
		echo "{" >> $TOOLCHAINS
		name=$(basename $gradleversion)
		path=$gradleversion
		echo "\"name\":\"$name\"," >> $TOOLCHAINS
		echo "\"path\":\"$path\"" >> $TOOLCHAINS
		echo "}" >>  $TOOLCHAINS
		last=$[$count-1]
		if [ $i -ne $last ]; then
			echo "," >>  $TOOLCHAINS
		fi		
	done
	echo "]}" >> $TOOLCHAINS
}

# probe python
probe_python()
{
	PYTHON_VERSIONS=($(find $search_path -name 'python[0-9]*\.*[0-9]*' | grep -P 'python[0-9]*\.*[0-9]*$'))
	echo "{\"Python\": [" >> $TOOLCHAINS
	count=${#PYTHON_VERSIONS[@]}
	for ((i=0;i<$count;i++)) do
		pythonversion=${PYTHON_VERSIONS[i]}
		echo "{" >> $TOOLCHAINS
		name=$(basename $pythonversion)
		path=$pythonversion
		echo "\"name\":\"$name\"," >> $TOOLCHAINS
		echo "\"path\":\"$path\"" >> $TOOLCHAINS
		echo "}" >>  $TOOLCHAINS
		last=$[$count-1]
		if [ $i -ne $last ]; then
			echo "," >>  $TOOLCHAINS
		fi		
	done
	echo "]}" >> $TOOLCHAINS
}

############################################################
# main entry
probe_c_compilers
echo "," >> $TOOLCHAINS
probe_cxx_compilers
echo "," >> $TOOLCHAINS
probe_debuggers
echo "," >> $TOOLCHAINS
probe_build_systems
echo "," >> $TOOLCHAINS
probe_lsp_servers
echo "," >> $TOOLCHAINS
probe_jdk
echo "," >> $TOOLCHAINS
probe_maven
echo "," >> $TOOLCHAINS
probe_gradle
echo "," >> $TOOLCHAINS
probe_python
echo "," >> $TOOLCHAINS
probe_ninja

echo "]" >> $TOOLCHAINS

exit 0
