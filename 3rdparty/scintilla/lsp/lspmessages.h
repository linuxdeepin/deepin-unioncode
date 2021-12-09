// Scintilla source code edit control
/** @file lspmessages.h
 ** Class for defining LSP messages used
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_MESSAGE_H
#define SCINTILLA_LSP_MESSAGE_H

#include "lspclientlogic.h"

#include "nlohmann/json.hpp"

namespace Scintilla {

//! Json LSP message is abstracted from library used
typedef nlohmann::json lspMessage;

class LspClientLogic;
struct LspDocInfo;
class LspMessageHandler;
struct LspPosition;

namespace LspMessages {

//! Type of the callback function for handling of answers
using HandleAnswerFunPtr = void(*)(uintptr_t id, LspClientLogic *, LspDocInfo *, const std::string &req, const lspMessage &asw);
//! Type of the callback function for handling of notifications from the server
using HandleServerNotifyFunPtr = void(*)(LspClientLogic *, const lspMessage &params);

//! Server initialization
class InitializeRequest
{
public:
	static uintptr_t send(LspMessageHandler *handler, const LspClientConfiguration &cfg);
	static void handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, const std::string &req, const lspMessage &result);
};


//! Request all document symbols
class DocumentSymbolsRequest
{
public:
	static uintptr_t send(LspMessageHandler *handler, LspDocInfo *info);
	static void handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, const std::string &req, const lspMessage &result);
};

class DocumentCompletionRequest
{
public:
	static uintptr_t send(LspMessageHandler *handler, LspDocInfo *info, const LspPosition &pos);
	static void handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, const std::string &req, const lspMessage &result);
};

class DocumentSignatureHelpRequest
{
public:
	static uintptr_t send(LspMessageHandler *handler, LspDocInfo *info, const LspPosition &pos);
	static void handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, const std::string &req, const lspMessage &result);
};

class DocumentHoverRequest
{
public:
	static uintptr_t send(LspMessageHandler *handler, LspDocInfo *info, const LspPosition &pos);
	static void handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, const std::string &req, const lspMessage &result);
};

//! Request server shutdown
class ShutdownRequest
{
public:
	static bool send(LspMessageHandler *handler);
	static void handleAnswer(uintptr_t id, LspClientLogic *logic, LspDocInfo *info, const std::string &req, const lspMessage &result);
};

}

namespace LspClientNotifications {

//! Notify document open
class DidOpenTextDocument
{
public:
	static bool send(LspMessageHandler *handler, const LspDocInfo &info);
};

//! Notify document close
class DidCloseTextDocument
{
public:
	static bool send(LspMessageHandler *handler, const LspDocInfo &info);
};

//! Notify document changes
class DidChangeTextDocument
{
public:
	static bool send(LspMessageHandler *handler, LspDocInfo &info, const SCNotification &scn);
};

//! Notify exit request
class ExitNotification
{
public:
	static bool send(LspMessageHandler *handler);
};

}

namespace LspServerNotifications
{

class PublishDiagnostics
{
public:
	static void handle(LspClientLogic *logic, const lspMessage &params);
};

class SemanticHighlighting
{
public:
	static void handle(LspClientLogic *logic, const lspMessage &params);
};

class ClangdFileStatus
{
public:
	static void handle(LspClientLogic *logic, const lspMessage &params);
};

}

}

#endif	// SCINTILLA_LSP_MESSAGE_H
