// Scintilla source code edit control
/** @file lspprocess.h
 ** Class for handling communication with child LSP process
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_LSP_PROCESS_H
#define SCINTILLA_LSP_PROCESS_H

#include "process.h"
#include "lspmessages.h"

namespace Scintilla
{

class LspMessageHandler;
class LspProcessPrivate;

//! Class to handle starting a new process and communicate with it through stdin/stdout
class LspProcess : public Process
{
public:
	LspProcess(LspMessageHandler *client);
	~LspProcess();

	int sendMessage(const std::string &msg);
	inline int sendMessage(const lspMessage &msg) { return sendMessage(msg.dump()); }

private:
	//! \called after process started
	void onProcessStarted() override;
	//! Called when some data is available from stdout of the child process
	void outDataAvailable(std::size_t sz) override;
	//! Called when a line of data is available from stdout of the child process
	void outLineAvailable() override;
	//! Function called after exit of the child process
	void onProcessExited(int exitcode) override;

private:
	//! Implementation
	LspProcessPrivate *const p;
};

}

#endif	// SCINTILLA_LSP_PROCESS_H
