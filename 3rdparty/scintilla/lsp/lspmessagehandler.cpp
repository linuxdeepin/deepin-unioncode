// Scintilla source code edit control
/** @file lspmessagehandler.cpp
 ** Class handling exchange of LSP messages
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "lspmessagehandler.h"
#include "lspmessages.h"
#include "lspprocess.h"

#include "../include/Platform.h"

using namespace Scintilla;

LspMessageHandler::LspMessageHandler(LspClientLogic *logic)
	: clientLogic(logic),
	proc(nullptr),
	deleteProc(false),
	lastId(0)
{
	serverNotifyHandlers.emplace("textDocument/publishDiagnostics", &LspServerNotifications::PublishDiagnostics::handle);
	serverNotifyHandlers.emplace("textDocument/semanticHighlighting", &LspServerNotifications::SemanticHighlighting::handle);
	serverNotifyHandlers.emplace("textDocument/clangd.fileStatus", &LspServerNotifications::ClangdFileStatus::handle);
}

LspMessageHandler::~LspMessageHandler()
{
	// Deferred server delete
	checkDeleteProc();
	// Stop the server
	if (procOk())
	{
		LspMessages::ShutdownRequest::send(this);
		waitServerProcessFinished();
	}
}

bool LspMessageHandler::startServer(const char *exec, const char *params)
{
	checkDeleteProc();
	if (proc == nullptr)
		proc = new LspProcess(this);
	deleteProc = false;
	proc->setExecutable(exec, params);
	proc->start();
	return true;
}

bool LspMessageHandler::isServerStarted() const
{
	return (procOk() && proc->isStarted());
}

void LspMessageHandler::waitServerProcessFinished()
{
	checkDeleteProc();
	if (!procOk())
		return;
	proc->waitProcessFinished();
	checkDeleteProc();
}

bool LspMessageHandler::sendMessage(std::uintptr_t id, LspDocInfo *info, const std::string &msg, LspMessages::HandleAnswerFunPtr callback)
{
	if (!procOk())
		return false;
	sentMessages.emplace(id, std::forward_as_tuple(msg, info, callback));
	return (proc->sendMessage(msg) > 0);
}

bool LspMessageHandler::sendMessage(std::uintptr_t id, LspDocInfo *info, const lspMessage &msg, LspMessages::HandleAnswerFunPtr callback)
{
	return sendMessage(id, info, msg.dump(), callback);
}

bool LspMessageHandler::sendNotification(const std::string &msg)
{
	if (!procOk())
		return false;
	return (proc->sendMessage(msg) > 0);
}

bool LspMessageHandler::sendNotification(const lspMessage &msg)
{
	return sendNotification(msg.dump());
}

void LspMessageHandler::onLspServerStarted()
{
	clientLogic->onServerStarted();
}

void LspMessageHandler::lspMessageReceived(const std::string &txt)
{
	const lspMessage msg = lspMessage::parse(txt);
	const auto &id_it = msg.find("id");
	if (id_it != msg.end())
	{
		const uintptr_t id = id_it->get<uintptr_t>();
		auto it_asw = sentMessages.find(id);
		if (it_asw != sentMessages.end())
		{
			const auto it_result = msg.find("result");
			const auto &req = std::get<std::string>(it_asw->second);
			auto callbackFun = std::get<LspMessages::HandleAnswerFunPtr>(it_asw->second);
			LspDocInfo *info = std::get<LspDocInfo *>(it_asw->second);
			const lspMessage result = (it_result == msg.end()) ? lspMessage() : std::move(it_result.value());
			sentMessages.erase(it_asw);
			if (callbackFun)
				callbackFun(id, clientLogic, info, req, result);
			return;		// Message handled
		}
	}
	// Message has no id: handle it
	if (msg.contains("method") && msg.contains("params"))
	{
		auto method_it = serverNotifyHandlers.find(msg.at("method"));
		if (method_it != serverNotifyHandlers.end())
		{
			const lspMessage params = msg.at("params");
			method_it->second(clientLogic, params);
			return;		// Message handled
		}
	}
	Platform::DebugPrintf("Got unhandled message from server: '%s'\n%s\n", txt.c_str(), msg.dump(4).c_str());
}

void LspMessageHandler::onLspProcessExited()
{
	deleteProc = true;
	sentMessages.clear();
}

void LspMessageHandler::execDeleteProc()
{
	delete proc;
	proc = nullptr;
}
