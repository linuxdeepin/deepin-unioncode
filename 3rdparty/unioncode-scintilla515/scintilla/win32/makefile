# Make file for Scintilla on Windows
# @file makefile
# Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile assumes Mingw-w64 GCC 9.0+ is used and changes will be needed to use other compilers.
# Clang 9.0+ can be used with CLANG=1 on command line.

.PHONY: all clean analyze depend

.SUFFIXES: .cxx .c .o .h .a

DIR_BIN=../bin

COMPONENT = $(DIR_BIN)/Scintilla.dll
LIBSCI = $(DIR_BIN)/libscintilla.a

WARNINGS = -Wpedantic -Wall

ifdef CLANG
CXX = clang++
DEFINES += -D_CRT_SECURE_NO_DEPRECATE
else
# MinGW GCC
LIBSMINGW = -lstdc++
STRIPOPTION = -s
endif
ARFLAGS = rc
RANLIB ?= ranlib
WINDRES ?= windres

# Environment variable windir always defined on Win32

# Take care of changing Unix style '/' directory separator to '\' on Windows
normalize = $(if $(windir),$(subst /,\,$1),$1)

PYTHON = $(if $(windir),pyw,python3)

ifdef windir
DEL = $(if $(wildcard $(dir $(SHELL))rm.exe), $(dir $(SHELL))rm.exe -f, del /q)
else
DEL = rm -f
endif

vpath %.h ../src ../include
vpath %.cxx ../src

LDFLAGS=-shared -static -mwindows
LIBS=-lgdi32 -luser32 -limm32 -lole32 -luuid -loleaut32 -lmsimg32 $(LIBSMINGW)

INCLUDES=-I ../include -I ../src

BASE_FLAGS += $(WARNINGS)

ifdef NO_CXX11_REGEX
DEFINES += -DNO_CXX11_REGEX
endif

DEFINES += -D$(if $(DEBUG),DEBUG,NDEBUG)
BASE_FLAGS += $(if $(DEBUG),-g,-Os)

ifndef DEBUG
STRIPFLAG=$(STRIPOPTION)
endif

CXX_BASE_FLAGS =--std=c++17 $(BASE_FLAGS)
CXX_ALL_FLAGS =$(DEFINES) $(INCLUDES) $(CXX_BASE_FLAGS)

all:	$(COMPONENT) $(LIBSCI)

clean:
	$(DEL) *.exe *.o *.a *.obj *.dll *.res *.map *.plist $(call normalize,$(LIBSCI))

%.o: %.cxx
	$(CXX) $(CXX_ALL_FLAGS) $(CXXFLAGS) -c $<

analyze:
	$(CXX) --analyze $(CXX_ALL_FLAGS) $(CXXFLAGS) *.cxx ../src/*.cxx

depend deps.mak:
	$(PYTHON) DepGen.py

# Required for base Scintilla
SRC_OBJS = \
	AutoComplete.o \
	CallTip.o \
	CaseConvert.o \
	CaseFolder.o \
	CellBuffer.o \
	CharacterCategoryMap.o \
	CharacterType.o \
	CharClassify.o \
	ContractionState.o \
	DBCS.o \
	Decoration.o \
	Document.o \
	EditModel.o \
	Editor.o \
	EditView.o \
	Geometry.o \
	Indicator.o \
	KeyMap.o \
	LineMarker.o \
	MarginView.o \
	PerLine.o \
	PositionCache.o \
	RESearch.o \
	RunStyles.o \
	Selection.o \
	Style.o \
	UniConversion.o \
	UniqueString.o \
	ViewStyle.o \
	XPM.o

COMPONENT_OBJS = \
	$(SRC_OBJS) \
	HanjaDic.o \
	PlatWin.o \
	ScintillaBase.o \
	ScintillaWin.o

SHARED_OBJS = \
	ScintillaDLL.o \
	ScintRes.o

$(COMPONENT): $(COMPONENT_OBJS) $(SHARED_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(STRIPFLAG) $^ $(CXXFLAGS) $(LIBS)

$(LIBSCI): $(COMPONENT_OBJS)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

# Automatically generate dependencies for most files with "make deps"
include deps.mak

ScintRes.o: ScintRes.rc
	$(WINDRES) $< $@

