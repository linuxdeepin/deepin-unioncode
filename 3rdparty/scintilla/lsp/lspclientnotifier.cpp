// Scintilla source code edit control
/** @file lspclientnotifier.cpp
 ** Definition of the interface for interacting with a client object for Language Server Protocol
 **/
// Copyright (c) 2019-2020 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "lspclientnotifier.h"
#include "ILspClientObject.h"

using namespace Scintilla;

LspClientNotifier::~LspClientNotifier()
{
	// Move object list to not get disturbed by the object calling unregisterObject()
	auto obj_copy = std::move(objects);
	for (auto it = obj_copy.begin(); it != obj_copy.end(); ++it)
	{
		auto *object = it->second;
		if (object) object->removeLspClient();
	}
}

void LspClientNotifier::registerObject(const LspScintillaDoc &doc, ILspClientObject *object)
{
	objects.emplace_back(std::make_pair(doc.getDoc(), object));
}

void LspClientNotifier::unregisterObject(const ILspClientObject *object)
{
	for (auto it = objects.begin(); it != objects.end(); )
	{
		if (it->second == object)
			it = objects.erase(it);
		else
			++it;
	}
}

void LspClientNotifier::registerRequest(uintptr_t id, ILspClientObject *object)
{
	requests.insert({id, object});
}

void LspClientNotifier::notifyLspDiagnostic(const LspScintillaDoc &doc, const LspDocumentDiagnostic &dd)
{
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		if (it->first == doc.getDoc())
			it->second->notifyLspDiagnostic(dd);
	}
}

void LspClientNotifier::notifyDocumentSymbols(uintptr_t id, const LspDocumentSymbols &s)
{
	const auto it = requests.find(id);
	if (it == requests.end()) return;
	it->second->notifyDocumentSymbols(s);
	requests.erase(it);
}

void LspClientNotifier::notifyLspHover(uintptr_t id, const LspHover &ho)
{
	const auto it = requests.find(id);
	if (it == requests.end()) return;
	it->second->notifyLspHover(ho);
	requests.erase(it);
}

void LspClientNotifier::notifyLspCompletion(uintptr_t id, const LspCompletionList &cl)
{
	const auto it = requests.find(id);
	if (it == requests.end()) return;
	it->second->notifyLspCompletion(cl);
	requests.erase(it);
}

void LspClientNotifier::notifyLspSignatureHelp(uintptr_t id, const LspSignatureHelp &sh)
{
	const auto it = requests.find(id);
	if (it == requests.end()) return;
	it->second->notifyLspSignatureHelp(sh);
	requests.erase(it);
}

void LspClientNotifier::notifyLspFileStatus(const LspScintillaDoc &doc, const LspClangdFileStatus &sts)
{
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		if (it->first == doc.getDoc())
			it->second->notifyLspFileStatus(sts);
	}
}
