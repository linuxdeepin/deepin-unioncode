// Scintilla source code edit control
/** @file lspstructures.cpp
 ** Class for converting from json objects to structures
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "lspstructures.h"
#include "base64.h"

#include "ILspDocument.h"
#include "../include/Platform.h"

using namespace Scintilla;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define LSP_GET(MSG, X) lspGetValue(X, MSG, #X)

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LspPosition::LspPosition(const LspScintillaDoc &doc, Sci_Position pos)
{
	line = int(doc.LineFromPosition(pos));
	const auto lpos_start = doc.LineStart(line);
	character = int(doc.CountUTF16(lpos_start, pos));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sci_Position lspConv::sciPos(const LspScintillaDoc &doc, int line, int character)
{
	const Sci_Position l_start = doc.LineStart(line);
	return doc.GetRelativePositionUTF16(l_start, character);
}

lspMessage lspConv::Position(const LspScintillaDoc &doc, Sci_Position pos)
{
	return LspPosition(doc, pos).toMsg();
}

lspMessage lspConv::Range(const LspScintillaDoc &doc, Sci_Position pos, sptr_t len)
{
	return lspMessage({
		{ "start", lspConv::Position(doc, pos) },
		{ "end", lspConv::Position(doc, pos + len) }
		});
}

bool lspConv::fromPos(const lspMessage &msg, const LspScintillaDoc &doc, Sci_Position &pos)
{
	const auto &it_line = msg.find("line");
	const auto &it_character = msg.find("character");
	if ((it_line != msg.end()) && (it_character != msg.end()))
	{
		const int line = it_line->get<int>();
		const int character = it_character->get<int>();
		pos = sciPos(doc, line, character);
		return (pos != INVALID_POSITION);
	}
	return false;
}

bool lspConv::fromRange(const lspMessage &msg, const LspScintillaDoc &doc, Sci_Position &p_start, Sci_Position &p_end)
{
	const auto &it_start = msg.find("start");
	const auto &it_end = msg.find("end");
	if ((it_start == msg.end()) || (it_end == msg.end()))
		return false;
	return fromPos(it_start.value(), doc, p_start) && fromPos(it_end.value(), doc, p_end);
}

bool lspConv::convertRange(const LspScintillaDoc &doc, const LspRange &range, Sci_Position &p_start, Sci_Position &p_end)
{
#pragma message("TODO: check limits")
	p_start = sciPos(doc, range.start);
	p_end = sciPos(doc, range.end);
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LspServerCapabilities::fromMsg(const lspMessage &msg)
{
	const auto &cap_it = msg.find("capabilities");
	if (cap_it == msg.end())
		return;
	const lspMessage &capabilities = cap_it.value();
	LSP_GET(capabilities, codeActionProvider);
	const auto &cp = capabilities.find("completionProvider");
	if (cp != capabilities.end())
	{
		lspGetValue(completionProvider.resolveProvider, cp.value(), "resolveProvider");
		const auto &tc_it = cp.value().find("triggerCharacters");
		completionProvider.triggerCharacters.clear();
		if (tc_it != cp->end())
			for (auto i : tc_it.value())
				completionProvider.triggerCharacters.append(i.get<std::string>());
	}
	LSP_GET(capabilities, declarationProvider);
	LSP_GET(capabilities, definitionProvider);
	LSP_GET(capabilities, documentFormattingProvider);
	LSP_GET(capabilities, documentHighlightProvider);
	const auto &dotfm = capabilities.find("documentOnTypeFormattingProvider");
	if (dotfm != capabilities.end())
	{
		const auto &ftc_it = dotfm->find("firstTriggerCharacter");
		if (ftc_it != dotfm->end())
			documentOnTypeFormattingProvider.firstTriggerCharacter = ftc_it->get<std::string>().at(0);
		//
		documentOnTypeFormattingProvider.moreTriggerCharacter.clear();
		const auto &mtc_it = dotfm->find("moreTriggerCharacter");
		if (mtc_it != dotfm->end())
			for (auto i : mtc_it.value())
				documentOnTypeFormattingProvider.moreTriggerCharacter.append(i.get<std::string>());
	}
	LSP_GET(capabilities, documentRangeFormattingProvider);
//	struct _executeCommandProvider
//	{
//		_executeCommandProvider() = default;
//		std::vector<std::string> commands;
//	} executeCommandProvider;
	LSP_GET(capabilities, hoverProvider);
	LSP_GET(capabilities, referencesProvider);
	LSP_GET(capabilities, renameProvider);
	LSP_GET(capabilities, selectionRangeProvider);
	const auto &semanticHighlighting_it = capabilities.find("semanticHighlighting");
	if ((semanticHighlighting = (semanticHighlighting_it != capabilities.end())))
	{
		const auto &scopes = semanticHighlighting_it->find("scopes");
		for (auto i : scopes.value())
		{
			if (!i.is_array())
				continue;
			for (auto j : i)
				if (j.is_string())
					semanticHighlightingScopes.push_back(j.get<std::string>());
		}
		for (int i = 0; i < semanticHighlightingScopes.size(); ++i)
			Platform::DebugPrintf("Semantic highlighting scope %2d '%s'", i, semanticHighlightingScopes.at(i).c_str());
	}
	const auto &signatureHelpProvider_it = capabilities.find("signatureHelpProvider");
	if ((signatureHelpProvider = (signatureHelpProvider_it != capabilities.end())))
		signatureHelpOptions.fromMsg(signatureHelpProvider_it.value());
	LSP_GET(capabilities, textDocumentSync);
	LSP_GET(capabilities, typeHierarchyProvider);
	LSP_GET(capabilities, workspaceSymbolProvider);
}

void Scintilla::LspServerCapabilities::LspSignatureHelpOptions::fromMsg(const lspMessage &options)
{
	const auto &tc_it = options.find("triggerCharacters");
	triggerCharacters.clear();
	if (tc_it != options.end())
		for (auto i : tc_it.value())
			triggerCharacters.append(i.get<std::string>());
}

bool LspSymbolDef::fromMsg(const lspMessage &msg)
{
	const auto &it_name = msg.find("name");
	const auto &it_kind = msg.find("kind");
	const auto &it_location = msg.find("location");
	const auto &it_containerName = msg.find("containerName");
	if ((it_name == msg.end()) || (it_kind == msg.end()) || (it_location == msg.end()))
		return false;
	kind = it_kind->get<int>();
	name = it_name->get<std::string>();
    containerName = (it_containerName == msg.end()) ? std::string() : it_containerName->get<std::string>();
	return location.fromMsg(it_location.value());
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspDocumentSymbols::fromMsg(const lspMessage &msg)
{
	clear();
	if (!msg.is_array())
		return false;
	for (auto symbol : msg)
		emplace_back(std::forward<const lspMessage &>(symbol));
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspDiagnosticRelatedInformation::fromMsg(const lspMessage &msg)
{
	const auto &it_location = msg.find("location");
	if (it_location == msg.end())
		return false;
	location.fromMsg(it_location.value());
	if (!LSP_GET(msg, message))
		return false;
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspDiagnosticElement::fromMsg(const lspMessage &msg)
{
	const auto &it_range = msg.find("range");
	if (it_range == msg.end())
		return false;
	range.fromMsg(it_range.value());
	LSP_GET(msg, severity);
	LSP_GET(msg, code);
	LSP_GET(msg, source);
	LSP_GET(msg, message);
	const auto &related_range = msg.find("relatedInformation");
	if (related_range != msg.end())
	{
		for (auto i : related_range.value())
			relatedInformation.emplace_back(i);
	}
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspDocumentDiagnostic::fromMsg(const lspMessage &msg)
{
	if (!msg.is_array())
		return false;
	for (auto element : msg)
	{
		push_back(LspDiagnosticElement());
		if (!back().fromMsg(element))
			return false;
	}
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LspTextEdit::fromMsg(const lspMessage &msg)
{
	const auto &it_range = msg.find("range");
	if (it_range != msg.end())
		range.fromMsg(it_range.value());
	LSP_GET(msg, newText);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LspCompletionItem::fromMsg(const lspMessage &msg)
{
	LSP_GET(msg, label);
	LSP_GET(msg, kind);
	LSP_GET(msg, detail);
	LSP_GET(msg, documentation);
	LSP_GET(msg, sortText);
	LSP_GET(msg, filterText);
	LSP_GET(msg, insertText);
	const auto &it_textEdit = msg.find("textEdit");
	if (it_textEdit != msg.end())
		textEdit.fromMsg(it_textEdit.value());
	// additionalTextEdits;
	// command;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LspCompletionList::fromMsg(const lspMessage &msg)
{
	// If field not present, default value is false --> list IS complete
	lspGetValue(isIncomplete, msg, "isIncomplete");
	// Result can be a CompletionList structure or an item list
	const auto &items_it = msg.find("items");
	const lspMessage &itemsList = (items_it == msg.end()) ? msg : items_it.value();
	for (auto item : itemsList)
	{
		items.emplace_back();
		items.back().fromMsg(item);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspParameterInformation::fromMsg(const lspMessage &msg)
{
	const auto &label_it = msg.find("label");
	if (label_it == msg.end())
		return false;
	if (label_it->is_string())
	{
		label = label_it->get<std::string>();
		label_start = label_end = -1;
	}
	else if (label_it->is_array() && label_it->size() == 2)
	{
		label.clear();
		label_start = (*label_it)[0].get<int>();
		label_end = (*label_it)[1].get<int>();
	}
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspSignatureInformation::fromMsg(const lspMessage &msg)
{
	if (!LSP_GET(msg, label))
		return false;
	LSP_GET(msg, documentation);
	const auto &parameters_it = msg.find("parameters");
	if (parameters_it == msg.end())
		return false;
	for (auto p : parameters_it.value())
		parameters.emplace_back(std::forward<const lspMessage &>(p));
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspSignatureHelp::fromMsg(const lspMessage &msg)
{
	LSP_GET(msg, activeSignature);
	LSP_GET(msg, activeParameter);
	const auto &signatures_it = msg.find("signatures");
	if (signatures_it == msg.end())
		return false;
	for (auto s : signatures_it.value())
		signatures.emplace_back(std::forward<const lspMessage &>(s));
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool SemanticHighlightingLineInfo::fromMsg(const lspMessage &msg)
{
	if (!LSP_GET(msg, line)) return false;
	if (!LSP_GET(msg, tokens)) return false;
	const auto &it_isInactive = msg.find("isInactive");
	if (it_isInactive == msg.end())
		isInactive = false;
	else
	{
		if (it_isInactive->is_string())
			isInactive = it_isInactive->get<std::string>() == "true";
		else if (it_isInactive->is_number())
			isInactive = it_isInactive->get<int>();
		else if (it_isInactive->is_boolean())
			isInactive = it_isInactive->get<bool>();
		else
			isInactive = false;
	}
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool hostIsBigEndian()
{
	const int test = 0xFF;
	return reinterpret_cast<const uint8_t *>(&test)[0] == 0;
}

template <class T> void byteswap(T &data)
{
	char *buf = reinterpret_cast<char *>(&data);
	for (int i = 0; i < sizeof(T) / 2; ++i)
		std::swap(buf[i], buf[sizeof(T) - i - 1]);
}

std::vector<LspSemanticToken> SemanticHighlightingLineInfo::decodeTokens() const
{
	if (tokens.size() == 0)
		return {};
	std::string decoded;
	if (!base64Decode(tokens, decoded))
		return {};
	const auto sz = decoded.size();
	if (sz % 8 != 0)
		return {};
	std::vector<LspSemanticToken> ret_v;
	for (int o = 0; o < sz / 8; o++)
	{
		const int base = o * 8;
		ret_v.emplace_back();
		auto &val = ret_v.back();
		val.character = reinterpret_cast<const uint32_t &>(decoded.at(base));
		val.length = reinterpret_cast<const uint16_t &>(decoded.at(base + 4));
		val.scope = reinterpret_cast<const uint16_t &>(decoded.at(base + 6));
		// Binary data transmitted is big endian: swap it if host is not
		if (!hostIsBigEndian())
		{
			byteswap(val.character);
			byteswap(val.length);
			byteswap(val.scope);
		}
	}
	return std::move(ret_v);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool LspSemanticHighlighting::fromMsg(const lspMessage &msg)
{
	const auto &it_textDocument = msg.find("textDocument");
	if (it_textDocument == msg.end())
		return false;
	const auto &it_lines = msg.find("lines");
	if (it_lines == msg.end())
		return false;
	identifier.fromMsg(it_textDocument.value());
	for (auto i : it_lines.value())
		lines.emplace_back(i);
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void LspDocumentSemanticHighlighting::clear()
{
	lines.clear();
}

void LspDocumentSemanticHighlighting::update(LspSemanticHighlighting &&data)
{
	if (lines.size() == 0)
	{
		lines = std::move(data.lines);
		return;
	}
	// Received an empty message: do not modify the list
	if (data.lines.size() == 0)
		return;
	//
	//	Scan the two arrays to update the lines
	//
	auto it_d = data.lines.begin();
	for (auto it_l = lines.begin(); ((it_l != lines.end()) && (it_d != data.lines.end())); ++it_l)
	{
		// Find line in data greater than or equal the actual line
		for (; (it_d != data.lines.end() && (it_d->line < it_l->line)); ++it_d)
		{
			// All lines in data with line number less than actual
			// will be inserted before actual line
			if (it_d->validLine())
				lines.insert(it_l, std::move(*it_d));
		}
		if ((it_d != data.lines.end()) && (it_d->line == it_l->line))
		{
			// Same line: move value
			*it_l = std::move(*it_d);
			++it_d;
		}
	}
	//
	//	Append data remaining in data.lines to lines
	//
	if (it_d != data.lines.end())
		std::move(it_d, data.lines.end(), std::back_inserter(lines));
	//
	//	Erase invalid lines
	//
	for (auto it_l = lines.begin(); it_l != lines.end();)
	{
		if (it_l->validLine())
			++it_l;
		else
			it_l = lines.erase(it_l);
	}
	//
	//	Empty data.lines
	//
	data.lines.clear();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool Scintilla::LspMarkupContent::fromMsg(const lspMessage &msg)
{
	if (!LSP_GET(msg, kind))
		return false;
	return LSP_GET(msg, value);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool Scintilla::LspHover::fromMsg(const lspMessage &msg)
{
	const auto &it_contents = msg.find("contents");
	if (it_contents == msg.end())
		return false;
	contents.fromMsg(it_contents.value());
	//
	const auto &it_range = msg.find("range");
	if (it_range != msg.end())
		range.fromMsg(it_range.value());
	return false;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool Scintilla::LspClangdFileStatus::fromMsg(const lspMessage &msg)
{
	if (!LSP_GET(msg, uri))
		return false;
	if (!LSP_GET(msg, state))
		return false;
	return true;
}
