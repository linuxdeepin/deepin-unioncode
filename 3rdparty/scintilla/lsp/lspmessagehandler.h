// Scintilla source code edit control
/** @file lspmessagehandler.h
 ** Class handling exchange of LSP messages
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_MESSAGEHANDLER_H
#define SCINTILLA_LSP_MESSAGEHANDLER_H

#include "lspmessages.h"

#include <map>

namespace Scintilla {

class LspProcess;

class LspMessageHandler
{
	friend class LspProcess;

public:
	LspMessageHandler(LspClientLogic *logic);
	~LspMessageHandler();

	//! Start executable of LSP server
	bool startServer(const char *exec, const char *params);
	//! Test logical status of server started
	bool isServerStarted() const;
	//! Wait for finish of the server process
	void waitServerProcessFinished();
	//! Get next id.
	std::uintptr_t nextId() { return (++lastId != 0 ? lastId : ++lastId); }
	//! Send a message and add a copy to the queue of sent messages
	bool sendMessage(std::uintptr_t id, LspDocInfo *info, const std::string &msg, LspMessages::HandleAnswerFunPtr callback = nullptr);
	//! Shortcut for sendMessage(std::uintptr_t, const std::string &);
	bool sendMessage(std::uintptr_t id, LspDocInfo *info, const lspMessage &msg, LspMessages::HandleAnswerFunPtr callback = nullptr);
	//! Send a notification: the server does not answer
	bool sendNotification(const std::string &msg);
	//! Send a notification: the server does not answer
	bool sendNotification(const lspMessage &msg);

protected:
	//! Called after server process started
	void onLspServerStarted();
	//! Got a message, interpret it
	void lspMessageReceived(const std::string &txt);
	//! Notification from server process
	void onLspProcessExited();

private:
	//! Check process is Ok
	inline bool procOk() const
	{
		return (!deleteProc) && (proc != nullptr);
	}
	//! Check deferred deltion of external process
	inline void checkDeleteProc()
	{
		if (proc && deleteProc) execDeleteProc();
	}
	//! Execute deletion of external process
	void execDeleteProc();

private:
	//! The object handling the client logic
	LspClientLogic *const clientLogic;
	//! The external process with which we communicate
	LspProcess *proc;
	//! Flag for deleting \ref proc : cannot delete process inside handler!
	bool deleteProc;
	//! Map of the handlers for notifications from the server
	std::map<std::string, LspMessages::HandleServerNotifyFunPtr> serverNotifyHandlers;
	//! Map of the messages waiting for an answer (key is message id, values are message, document info and handler function)
	std::map<uintptr_t, std::tuple<std::string, LspDocInfo *, LspMessages::HandleAnswerFunPtr>> sentMessages;
	//! Last used id
	std::uintptr_t lastId;
};

}

#endif	// SCINTILLA_LSP_MESSAGEHANDLER_H
