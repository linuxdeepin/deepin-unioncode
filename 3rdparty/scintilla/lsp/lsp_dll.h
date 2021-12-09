// Scintilla source code edit control
/** @file lsp_dll.h
 ** Definitions for import/export
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_DLL
#	ifdef WIN32
#		ifdef SCI_LSP_MAKE_LIBRARY
#			define SCINTILLA_LSP_DLL __declspec(dllexport)
#		else
#			define SCINTILLA_LSP_DLL __declspec(dllimport)
#		endif
#	else
#		define SCINTILLA_LSP_DLL
#	endif
#endif
