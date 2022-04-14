#!/bin/sh

#
#  Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
#
#    Author:     xiaozaihu<xiaozaihu@uniontech.com>
# 
#    Maintainer: xiaozaihu<xiaozaihu@uniontech.com>
#                
#
#  This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
#  Init The Native Runtime Environments For UnionCode IDE
#
export UNIONCODE_VERSION="1.0.0"
export UNIONCODE_LIBEXEC="/usr/libexec/unioncode/"

##################################################################################
# probe gcc
GCC_COMPILER=""
probe_gcc()
{
	pkg="gcc"
	gcc_c_compiler_path=""
	gcc_cpp_compiler_path=""
       	gcc_c_compiler_version=""
	gcc_cpp_compiler_version="" 
	if ! command -v $pkg >/dev/null; then
		apt install $pkg -y
	fi
	gcc_c_compiler_version=$(gcc --version | head -1)
	gcc_c_compiler_path=$(command -v gcc)
	gcc_cpp_compiler_version=$(gcc --version | head -1)
	gcc_cpp_compiler_path=$(command -v g++)	

	GCC_C_COMPILER=$(
cat <<-EOD
{
  "name": "GCC C COMPILER in $gcc_c_compiler_path",
  "version": "$gcc_c_compiler_version",
  "command": "gcc",
  "path": "$gcc_c_compiler_path"
}
EOD
)
	GCC_CPP_COMPILER=$(
cat <<-EOD
{
  "name": "GCC CPP COMPILER in $gcc_cpp_compiler_path",
  "version": "$gcc_cpp_compiler_version",
  "command": "g++",
  "path": "$gcc_cpp_compiler_path"
}
EOD
)

	GCC_COMPILER=$(
cat <<-EOD
[
    $GCC_C_COMPILER,
    $GCC_CPP_COMPILER
]
EOD
)
	#echo "$GCC_COMPILER"
}

# probe x86_64-linux-gcc
X86_64_LINUX_GCC_COMPILER=""
probe_amd64_gcc()
{
	pkg="crossbuild-essential-amd64"
	x86_64_linux_gcc_c_compiler_path=""
       	x86_64_linux_gcc_cpp_compiler_path=""
       	x86_64_linux_gcc_c_compiler_version=""
	x86_64_linux_gcc_cpp_compiler_version="" 
	if ! command -v $pkg >/dev/null; then
		apt install $pkg -y
	fi
	x86_64_linux_gcc_c_compiler_version=$(x86_64-linux-gnu-gcc --version | head -1)
	x86_64_linux_gcc_c_compiler_path=$(command -v x86_64-linux-gnu-gcc)
	x86_64_linux_gcc_cpp_compiler_version=$(x86_64-linux-gnu-gcc --version | head -1)
	x86_64_linux_gcc_cpp_compiler_path=$(command -v x86_64-linux-gnu-g++)	

	X86_64_LINUX_GCC_C_COMPILER=$(
cat <<-EOD
{
  "name": "GCC C COMPILER in $x86_64_linux_gcc_c_compiler_path",
  "version": "$x86_64_linux_gcc_c_compiler_version",
  "command": "x86_64-linux-gnu-gcc",
  "path": "$x86_64_linux_gcc_c_compiler_path"
}
EOD
)
	X86_64_LINUX_GCC_CPP_COMPILER=$(
cat <<-EOD
{
  "name": "GCC CPP COMPILER in $x86_64_linux_gcc_cpp_compiler_path",
  "version": "$x86_64_linux_gcc_cpp_compiler_version",
  "command": "x86_64-linux-gnu-g++",
  "path": "$x86_64_linux_gcc_cpp_compiler_path"
}
EOD
)

	X86_64_LINUX_GGCC_COMPILER=$(
cat <<-EOD
[
    $X86_64_LINUX_GCC_C_COMPILER,
    $X86_64_LINUX_GCC_CPP_COMPILER
]
EOD
)
#	echo "$X86_64_LINUX_GCC_COMPILER"
}

# probe aarch64-linux-gcc
AARCH64_LINUX_GCC_COMPILER=""
probe_aarch64_gcc()
{
	pkg="crossbuild-essential-arm64"
	aarch64_linux_gcc_c_compiler_path=""
       	aarch64_linux_gcc_cpp_compiler_path=""
       	aarch64_linux_gcc_c_compiler_version=""
	aarch64_linux_gcc_cpp_compiler_version="" 
	if ! command -v $pkg >/dev/null; then
		apt install $pkg -y
	fi
	aarch64_linux_gcc_c_compiler_version=$(aarch64-linux-gnu-gcc --version | head -1)
	aarch64_linux_gcc_c_compiler_path=$(command -v aarch64-linux-gnu-gcc)
	aarch64_linux_gcc_cpp_compiler_version=$(aarch64-linux-gnu-gcc --version | head -1)
	aarch64_linux_gcc_cpp_compiler_path=$(command -v aarch64-linux-gnu-g++)	

	AARCH64_LINUX_GCC_C_COMPILER=$(
cat <<-EOD
{
  "name": "GCC C COMPILER in $aarch64_linux_gcc_c_compiler_path",
  "version": "$aarch64_linux_gcc_c_compiler_version",
  "command": "aarch64-linux-gnu-gcc",
  "path": "$aarch64_linux_gcc_c_compiler_path"
}
EOD
)
	AARCH64_LINUX_GCC_CPP_COMPILER=$(
cat <<-EOD
{
  "name": "GCC CPP COMPILER in $aarch64_linux_gcc_cpp_compiler_path",
  "version": "$aarch64_linux_gcc_cpp_compiler_version",
  "command": "aarch64-linux-gnu-g++",
  "path": "$aarch64_linux_gcc_cpp_compiler_path"
}
EOD
)

	AARCH64_LINUX_GGCC_COMPILER=$(
cat <<-EOD
[
    $AARCH64_LINUX_GCC_C_COMPILER,
    $AARCH64_LINUX_GCC_CPP_COMPILER
]
EOD
)
#	echo "$AARCH64_LINUX_GCC_COMPILER"
}

# probe mips64el-linux-gcc
MIPS64EL_LINUX_GCC_COMPILER=""
probe_mips64el_gcc()
{
	pkg="crossbuild-essential-mips64el"
	mips64el_linux_gcc_c_compiler_path=""
       	mips64el_linux_gcc_cpp_compiler_path=""
       	mips64el_linux_gcc_c_compiler_version=""
	mips64el_linux_gcc_cpp_compiler_version="" 
	if ! command -v $pkg >/dev/null; then
		apt install $pkg -y
	fi
	mips64el_linux_gcc_c_compiler_version=$(mips64el-linux-gnuabi64-gcc --version | head -1)
	mips64el_linux_gcc_c_compiler_path=$(command -v mips64el-linux-gnuabi64-gcc)
	mips64el_linux_gcc_cpp_compiler_version=$(mips64el-linux-gnuabi64-gcc --version | head -1)
	mips64el_linux_gcc_cpp_compiler_path=$(command -v mips64el-linux-gnuabi64-g++)	

	MIPS64EL_LINUX_GCC_C_COMPILER=$(
cat <<-EOD
{
  "name": "GCC C COMPILER in $mips64el_linux_gcc_c_compiler_path",
  "version": "$mips64el_linux_gcc_c_compiler_version",
  "command": "mips64el-linux-gnuabi64-gcc",
  "path": "$mips64el_linux_gcc_c_compiler_path"
}
EOD
)
	MIPS64EL_LINUX_GCC_CPP_COMPILER=$(
cat <<-EOD
{
  "name": "GCC CPP COMPILER in $mips64el_linux_gcc_cpp_compiler_path",
  "version": "$mips64el_linux_gcc_cpp_compiler_version",
  "command": "mips64el-linux-gnuabi64-g++",
  "path": "$mips64el_linux_gcc_cpp_compiler_path"
}
EOD
)

	MIPS64EL_LINUX_GGCC_COMPILER=$(
cat <<-EOD
[
    $MIPS64EL_LINUX_GCC_C_COMPILER,
    $MIPS64EL_LINUX_GCC_CPP_COMPILER
]
EOD
)
#	echo "$MIPS64EL_LINUX_GCC_COMPILER"
}

# probe gdb
GDB_DEBUGGER=""
probe_gdb()
{
	pkg="gdb"
	gdb_debugger_path=""
	gdb_debugger_version=""

	if ! command -v $pkg >/dev/null; then
		apt install $pkg -y
	fi
	gdb_debugger_version=$(gdb --version | head -1)
	gdb_debugger_path=$(command -v gcc)

	GDB_DEBUGGER=$(
cat <<-EOD
{
  "name": "GDB DEBUGGER in $gdb_debugger_path",
  "version": "$gdb_debugger_version",
  "command": "gdb",
  "path": "$gdb_debugger_path"
}
EOD
)
	echo "$GDB_DEBUGGER"
}

# probe asmembler
GNU_ASMEMBLER=""
probe_as()
{
	ASMEMBLER_VERSION="$(as --version | head -1)"
	ASMEMBLER_PATH="$(command -v as)"
	GNU_ASMEMBLER=$(
cat <<-EOD
{
   "name": "GNU Asmembler as in $ASMEMBLER_PATH",
   "command": "as",
   "version": "$ASMEMBLER_VERSION",
   "path": "$ASMEMBLER_PATH"
}
EOD
)
	echo "$GNU_ASMEMBLER"
}

# probe x86_64-linux asmembler
X86_64_LINUX_GNU_ASMEMBLER=""
probe_amd64_as()
{
	ASMEMBLER_VERSION="$(x86_64-linux-gnu-as --version | head -1)"
	ASMEMBLER_PATH="$(command -v x86_64-linux-gnu-as)"
	X86_64_LINUX_GNU_ASMEMBLER=$(
cat <<-EOD
{
   "name": "GNU Asmembler as in $ASMEMBLER_PATH",
   "command": "x86_64-linux-gnu-as",
   "version": "$ASMEMBLER_VERSION",
   "path": "$ASMEMBLER_PATH"
}
EOD
)
#	echo "$X86_64_LINUX_GNU_ASMEMBLER"
}

# probe aarch64-linux asmembler
AARCH64_LINUX_GNU_ASMEMBLER=""
probe_aarch64_as()
{
	ASMEMBLER_VERSION="$(aarch64-linux-gnu-as --version | head -1)"
	ASMEMBLER_PATH="$(command -v aarch64-linux-gnu-as)"
	AARCH64_LINUX_GNU_ASMEMBLER=$(
cat <<-EOD
{
   "name": "GNU Asmembler as in $ASMEMBLER_PATH",
   "command": "aarch64-linux-gnu-as",
   "version": "$ASMEMBLER_VERSION",
   "path": "$ASMEMBLER_PATH"
}
EOD
)
#	echo "$AARCH64_LINUX_GNU_ASMEMBLER"
}

# probe mips64el-linux asmembler
MIPS64EL_LINUX_GNU_ASMEMBLER=""
probe_mips64el_as()
{
	ASMEMBLER_VERSION="$(mips64el-linux-gnuabi64-as --version | head -1)"
	ASMEMBLER_PATH="$(command -v mips64el-linux-gnuabi64-as)"
	MIPS64EL_LINUX_GNU_ASMEMBLER=$(
cat <<-EOD
{
   "name": "GNU Asmembler as in $ASMEMBLER_PATH",
   "command": "mips64el-linux-gnuabi64-as",
   "version": "$ASMEMBLER_VERSION",
   "path": "$ASMEMBLER_PATH"
}
EOD
)
#	echo "$MIPS64EL_LINUX_GNU_ASMEMBLER"
}

# probe linker
GNU_LINKER=""
probe_ld()
{
	LINKER_VERSION="$(ld --version | head -1)"
	LINKER_PATH="$(command -v ld)"
	GNU_LINKER=$(
cat <<-EOD
{
  "name": "GNU Linker ld in $LINKER_PATH",
  "command": "ld",
  "version": "$LINKER_VERSION",
  "path": "$LINKER_PATH"
}
EOD
)
#	echo "$GNU_LINKER"
}

# probe x86_64-linux linker
X86_64_LINUX_GNU_LINKER=""
probe_amd64_ld()
{
	LINKER_VERSION="$(x86_64-linux-gnu-ld --version | head -1)"
	LINKER_PATH="$(command -v x86_64-linux-gnu-ld)"
	X86_64_LINUX_GNU_LINKER=$(
cat <<-EOD
{
  "name": "GNU Linker ld in $LINKER_PATH",
  "command": "x86_64-linux-gnu-ld",
  "version": "$LINKER_VERSION",
  "path": "$LINKER_PATH"
}
EOD
)
#	echo "$X86_64_LINUX_GNU_LINKER"
}

# probe aarch64-linux linker
AARCH64_LINUX_GNU_LINKER=""
probe_aarch64_ld()
{
	LINKER_VERSION="$(aarch64-linux-gnu-ld --version | head -1)"
	LINKER_PATH="$(command -v aarch64-linux-gnu-ld)"
	AARCH64_LINUX_GNU_LINKER=$(
cat <<-EOD
{
  "name": "GNU Linker ld in $LINKER_PATH",
  "command": "aarch64-linux-gnu-ld",
  "version": "$LINKER_VERSION",
  "path": "$LINKER_PATH"
}
EOD
)
#	echo "$AARCH64_LINUX_GNU_LINKER"
}

# probe MIPS64el linux linker
MIPS64EL_LINUX_GNU_LINKER=""
probe_mips64el_ld()
{
	LINKER_VERSION="$(mips64el-linux-gnuabi64-ld --version | head -1)"
	LINKER_PATH="$(command -v mips64el-linux-gnuabi64-ld)"
	MIPS64EL_LINUX_GNU_LINKER=$(
cat <<-EOD
{
  "name": "GNU Linker ld in $LINKER_PATH",
  "command": "mips64-linux-gnuabi64-ld",
  "version": "$LINKER_VERSION",
  "path": "$LINKER_PATH"
}
EOD
)
#	echo "$MIPS64EL_LINUX_GNU_LINKER"
}

# probe glibc 
GNU_GLIBC=""
probe_glibc()
{
	GLIBC_VERSION="$(ldd --version | head -1)"
	GLIBC_PATH="$(command -v ldd)"
	GNU_GLIBC=$(
cat <<-EOD
{
   "name": "GNU Glibc ldd in $GLIBC_PATH",
   "command": "ldd",
   "version": "$GLIBC_VERSION",
   "path": "$GLIBC_PATH"
}
EOD
)
#	echo $GNU_GLIBC
}

# probe amd glibc 
X86_64_LINUX_GNU_GLIBC=""
probe_amd64_glibc()
{
	GLIBC_VERSION="$(x86_64-linux-gnu-ldd --version | head -1)"
	GLIBC_PATH="$(command -v x86_64-linux-gnu-ldd)"
	GNU_GLIBC=$(
cat <<-EOD
{
   "name": "GNU Glibc ldd in $GLIBC_PATH",
   "command": "x86_64-linux-gnu-ldd",
   "version": "$GLIBC_VERSION",
   "path": "$GLIBC_PATH"
}
EOD
)
#	echo $X86_64_LINUX_GNU_GLIBC
}

# probe arm64 glibc 
AARCH64_LINUX_GNU_GLIBC=""
probe_arm64_glibc()
{
	GLIBC_VERSION="$(aarch64-linux-gnu-ldd --version | head -1)"
	GLIBC_PATH="$(command -v aarch64-linux-gnu-ldd)"
	AARCH64_LINUX_GNU_GLIBC=$(
cat <<-EOD
{
   "name": "GNU Glibc ldd in $GLIBC_PATH",
   "command": "aarch64-linux-gnu-ldd",
   "version": "$GLIBC_VERSION",
   "path": "$GLIBC_PATH"
}
EOD
)
#	echo $AARCH64_LINUX_GNU_GLIBC
}

# probe mips64el glibc 
MIPS64EL_LINUX_GNU_GLIBC=""
probe_mips64el_glibc()
{
	GLIBC_VERSION="$(mips64el-linux-gnuabi64-ldd --version | head -1)"
	GLIBC_PATH="$(command -v mips64el-linux-gnuabi64-ldd)"
	MIPS64EL_LINUX_GNU_GLIBC=$(
cat <<-EOD
{
   "name": "GNU Glibc ldd in $GLIBC_PATH",
   "command": "mips64el-linux-gnuabi64-ldd",
   "version": "$GLIBC_VERSION",
   "path": "$GLIBC_PATH"
}
EOD
)
#	echo $MIPS64EL_LINUX_GNU_GLIBC
}

# probe make 
GNU_MAKE=""
probe_make()
{
  gnu_make_version="$(make --version | head -1)"
  gnu_make_path="$(command -v make)"

  GNU_MAKE=$(
cat <<-EOD
{
   "name": "gnu make in $gnu_make_path",
   "version": "$gnu_make_version",
   "command": "make",
   "path": "$gnu_make_path"
}
EOD
)
}

# scan gnu toolchain
GNU_TOOLCHAIN=""
scan_gnu_toolchain()
{
	# probe gcc
	probe_gcc
	COMPILER="$GCC_COMPILER"

	# probe gdb
	probe_gdb
	DEBUGGER="$GDB_DEBUGGER"

	# probe as
	probe_as
	ASMEMBLER="$GNU_ASMEMBLER"

	# probe ld
	probe_ld
	LINKER="$GNU_LINKER"	

	# probc glibc
	probe_glibc
	GLIBC="$GNU_GLIBC"

	# probe make
	probe_make
	MAKE=$GNU_MAKE

	GNU_TOOLCHAIN=$(
cat <<-EOD
{
  "name": "gnu linux toolchain",
  "arch": "x86_64",
  "toolchain":
  [ 
       {"asmembler": $ASMEMBLER}, 
       {"linker": $LINKER},
       {"glibc": $GLIBC},
       {"compiler": $COMPILER},
       {"debugger": $DEBUGGER},
       {"make": $MAKE}
  ] 
}
EOD
)
	#echo "$GNU_TOOLCHAIN"
}

# scan gnu cross x86_64 toolchain
CROSS_AMD64_GNU_TOOLCHAIN=""
scan_cross_amd64_gnu_toolchain()
{
	# probe gcc
	probe_amd64_gcc
	COMPILER="$X86_64_LINUX_GCC_COMPILER"

	# probe gdb
	probe_gdb
	DEBUGGER="$GDB_DEBUGGER"

	# probe as
	probe_amd64_as
	ASMEMBLER="$X86_64_LINUX_GNU_ASMEMBLER"

	# probe ld
	probe_amd64_ld
	LINKER="$X86_64_LINUX_GNU_LINKER"	

	# probc glibc
	probe_amd64_glibc
	GLIBC="$X86_64_LINUX_GNU_GLIBC"

	# probe make
	probe_make
	MAKE=$GNU_MAKE

	CROSS_AMD64_GNU_TOOLCHAIN=$(
cat <<-EOD
{
  "name": "cross gnu linux toolchain",
  "arch": "x86_64",
  "toolchain":
  [ 
       {"asmembler": $ASMEMBLER}, 
       {"linker": $LINKER},
       {"glibc": $GLIBC},
       {"compiler": $COMPILER},
       {"debugger": $DEBUGGER},
       {"make": $MAKE}
  ] 
}
EOD
)
	echo "$CROSS_AMD64_GNU_TOOLCHAIN"
}

# scan gnu cross arm64 toolchain
CROSS_AARCH64_GNU_TOOLCHAIN=""
scan_cross_aarch64_gnu_toolchain()
{
	# probe gcc
	probe_aarch64_gcc
	COMPILER="$AARCH64_LINUX_GCC_COMPILER"

	# probe gdb
	probe_gdb
	DEBUGGER="$GDB_DEBUGGER"

	# probe as
	probe_aarch64_as
	ASMEMBLER="$AARCH64_LINUX_GNU_ASMEMBLER"

	# probe ld
	probe_aarch64_ld
	LINKER="$AARCH64_LINUX_GNU_LINKER"	

	# probc glibc
	probe_aarch64_glibc
	GLIBC="$AARCH64_LINUX_GNU_GLIBC"

	# probe make
	probe_make
	MAKE=$GNU_MAKE

	CROSS_AARCH64_GNU_TOOLCHAIN=$(
cat <<-EOD
{
  "name": "cross gnu linux toolchain",
  "arch": "arm64",
  "toolchain":
  [ 
       {"asmembler": $ASMEMBLER}, 
       {"linker": $LINKER},
       {"glibc": $GLIBC},
       {"compiler": $COMPILER},
       {"debugger": $DEBUGGER},
       {"make": $MAKE}
  ] 
}
EOD
)
	echo "$CROSS_AARCH64_GNU_TOOLCHAIN"
}


# scan gnu mips64el toolchain
CROSS_MIPS64EL_GNU_TOOLCHAIN=""
scan_cross_mips64_gnu_toolchain()
{
	# probe gcc
	probe_mips64el_gcc
	COMPILER="$MIPS64EL_LINUX_GCC_COMPILER"

	# probe gdb
	probe_gdb
	DEBUGGER="$GDB_DEBUGGER"

	# probe as
	probe_mips64el_as
	ASMEMBLER="$MIPS64EL_LINUX_GNU_ASMEMBLER"

	# probe ld
	probe_mips64el_ld
	LINKER="$MIPS64EL_LINUX_GNU_LINKER"	

	# probc glibc
	probe_mips64el_glibc
	GLIBC="$MIPS64EL_LINUX_GNU_GLIBC"

	# probe make
	probe_make
	MAKE=$GNU_MAKE

	CROSS_MIPS64EL_GNU_TOOLCHAIN=$(
cat <<-EOD
{
  "name": "cross gnu linux toolchain",
  "arch": "mips64el",
  "toolchain":
  [ 
       {"asmembler": $ASMEMBLER}, 
       {"linker": $LINKER},
       {"glibc": $GLIBC},
       {"compiler": $COMPILER},
       {"debugger": $DEBUGGER},
       {"make": $MAKE}
  ] 
}
EOD
)
	echo "$CROSS_MIPS64EL_GNU_TOOLCHAIN"
}


###########################################################
# probe clang
LLVM_CLANG=""
probe_clang()
{
	pkg="clang"
	if ! command -v clang >/dev/null; then
		apt install -y clang
	fi

	llvm_clang_version="$(clang --version | head -1)"
	llvm_clang_path="$(command -v clang)"

	LLVM_CLANG=$(
cat <<-EOD
{
   "name": "LLVM clang compiler in $llvm_clang_path",
   "version": "$llvm_clang_version",
   "command": "clang",
   "path": "$llvm_clang_path"
}	
EOD
)
	#echo "$LLVM_CLANG"
}

# probe clang++
LLVM_CLANG_CPP=""
probe_clang_cpp()
{
	llvm_clang_cpp_version="$(clang++ --version | head -1)"
	llvm_clang_cpp_path="$(command -v clang++)"

	LLVM_CLANG_CPP=$(
cat <<-EOD
{
   "name": "LLVM clang++ compiler in $llvm_clang_cpp_path",
   "version": "$llvm_clang_cpp_version",
   "command": "clang++",
   "path": "$llvm_clang_cpp_path"
}
EOD
)
	#echo "$LLVM_CLANG_CPP"
}

# probe lldb
LLVM_LLDB=""
probe_lldb()
{
	pkg="lldb"
	if ! command -v lldb >/dev/null; then
		apt install -y lldb
	fi

	llvm_lldb_version="$(lldb --version | head -1)"
	llvm_lldb_path="$(command -v lldb)"
	LLVM_LLDB=$(
cat <<-EOD
{
   "name": "LLVM LLDB debugger in $llvm_lldb_path",
   "version": "$llvm_lldb_version",
   "command": "lldb",
   "path": "$llvm_lldb_path"
}
EOD
)
	#echo "$LLVM_LLDB"
}

# probe lld linker
LLVM_LLD=""
probe_lld()
{
	if ! command -v lld; then
		apt install -y lld
	fi
	lld_version="$(ld.lld --version | head -1)"
	lld_path="$(command -v ld.lld)"

	LLVM_LLD=$(
cat <<-EOD
{
   "name": "LLVM Linker in $lld_path",
   "version": "$lld_version",
   "command": "ld.lld",
   "path": "$lld_path"
}
EOD
)
	#echo "$LLVM_LLD"
}

# scan LLVM Toolchain
LLVM_TOOLCHAIN=""
scan_llvm_toolchain()
{
	# prob clang
	probe_clang
	CLANG="$LLVM_CLANG"

	# prob clang++
	probe_clang_cpp
	CLANG_CPP="$LLVM_CLANG_CPP"
	
	# prob lldb
	probe_lldb
	LLDB="$LLVM_LLDB"

	# prob ld.lld
	probe_lld
	LLD="$LLVM_LLD"

	LLVM_TOOLCHAIN=$(
cat <<-EOD
{
   "name": "LLVM Toolchain",
   "arch": "x86_64",
   "toolchain": [
      {"compiler":[$CLANG, $CLANG_CPP]},
      {"debugger": $LLDB},
      {"linker":  $LLD}, 
      {"make": $GNU_MAKE}
   ]
}
EOD
)
	#echo "$LLVM_TOOLCHAIN"
}

#############################################
# probe cmake
CMAKE_TOOL=""
probe_cmake()
{
	if ! command -v cmake >/dev/null; then
		apt install -y cmake
	fi

	cmake_version="$(cmake --version | head -1)"
	cmake_path="$(command -v cmake)"

	CMAKE_TOOL=$(
cat <<-EOD
{
  "name": "CMake tool in $cmake_path",
  "version": "$cmake_version",
  "command": "cmake",
  "path": "$cmake_path"
}
EOD
)
	#echo CMAKE_TOOL
}

# probe LSP server clangd
LSP_CLANGD=""
probe_lsp_clangd()
{
	if ! command -v clangd >/dev/null; then
		apt install clang-tools -y
	fi

	clangd_version="$(clangd --version | head -1)"
	clangd_path="$(command -v clangd)"

	LSP_CLANGD=$(
cat <<-EOD
{
   "name": "LSP server clangd in $clangd_path",
   "version": "$clangd_version",
   "command": "clangd",
   "path": "$clangd_path"
}
EOD
)

}

# probe ninja-build
NINJA_BUILD=""
probe_ninja_build()
{
	if ! command -v ninja >/dev/null; then
		apt install -y ninja-build
	fi

	ninja_version="$(ninja --version | head -1)"
	ninja_path="$(command -v ninja)"

	NINJA_BUILD=$(
cat <<-EOD
{
   "name": "ninja build in $ninja_path",
   "version": "$ninja_version",
   "command": "ninja",
   "path": "$ninja_path"
}
EOD
)	
}

# scan toolchains
TOOLCHAINS=""
scan_toolchains()
{
	scan_gnu_toolchain
	scan_cross_amd64_gnu_toolchain
	scan_cross_aarch64_gnu_toolchain
	scan_cross_mips64_gnu_toolchain
	scan_llvm_toolchain
	
	# probe cmake
	probe_cmake

	# probe LSP clangd
	probe_lsp_clangd
	
	# probe ninja build
	probe_ninja_build

	TOOLCHAINS=$(
cat <<-EOD
{
   "lsp": $LSP_CLANGD,
   "cmake": $CMAKE_TOOL,   
   "ninja": $NINJA_BUILD,
   "toolchains": [
      {"toolchain": $GNU_TOOLCHAIN},
      {"toolchain": $CROSS_AMD64_GNU_TOOLCHAIN},
      {"toolchain": $CROSS_AARCH64_GNU_TOOLCHAIN},
      {"toolchain": $CROSS_MIPS64EL_GNU_TOOLCHAIN},      
      {"toolchain": $LLVM_TOOLCHAIN}
   ]   
}
EOD
)

}

# main entry
scan_toolchains

export UNIONCODE_CONFIG="$HOME/.config/unioncode/configures"
# export UNIONCODE_CACHE="$HOME/.cache/unioncode"
if [ ! -d "$UNIONCODE_CONFIG" ] ; then
	mkdir -pv "$UNIONCODE_CONFIG"
fi

# if [ ! -d "$UNIONCODE_CACHE" ]; then
# 	mkdir -pv "$UNIONCODE_CACHE"
# fi

echo "$TOOLCHAINS" > "$UNIONCODE_CONFIG"/toolchains_cxx.support

exit 0
