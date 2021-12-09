// Scintilla source code edit control
/** @file lspprocess.cpp
 ** Class for handling communication with child LSP process
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "lspprocess.h"
#include "lspmessagehandler.h"
#include "lspmessages.h"
#include "../include/Platform.h"

#include <mutex>
#include <string_view>

using namespace Scintilla;

enum MessageParsePhase
{
	phaseReadContent,
	phaseReadMessage,
};

class Scintilla::LspProcessPrivate
{
public:
	LspProcessPrivate(LspMessageHandler *c) : handler(c), phase(phaseReadContent), contentLength(0), offset(0) {}
	~LspProcessPrivate() {}

	LspMessageHandler *const handler;
	std::mutex sendMutex;
	MessageParsePhase phase;
	std::string content;
	std::size_t contentLength;
	std::size_t offset;
};

LspProcess::LspProcess(LspMessageHandler *c)
	: p(new LspProcessPrivate(c))
{
}

LspProcess::~LspProcess()
{
	delete p;
}

int LspProcess::sendMessage(const std::string &msg)
{
	// Prepare header
	std::string hdr = {"Content-Length: "};
	hdr.append(std::to_string(msg.length()));
	hdr.append("\r\n\r\n");
	p->sendMutex.lock();
	// Write header
	writeToStdin(hdr.data(), hdr.length());
	// Write data
	const int result = writeToStdin(msg.data(), msg.length());
	p->sendMutex.unlock();
	return result;
}

void LspProcess::onProcessStarted()
{
	p->handler->onLspServerStarted();
}

void LspProcess::outDataAvailable(std::size_t sz)
{
	while (p->phase == phaseReadContent)
	{
		if (!canReadLineStdOut())
			return;
		outLineAvailable();
		if (p->phase == phaseReadMessage)
		{
			const auto to_read = std::min(sz, p->contentLength - p->offset);
			const std::size_t read = std::size_t(readFromStdout(&p->content[p->offset], to_read));
			p->offset += read;
			if (p->offset == p->contentLength)
			{
				// Message ready
				p->handler->lspMessageReceived(p->content);
				p->phase = phaseReadContent;
				p->offset = 0;
			}
		}
	}
}

void LspProcess::outLineAvailable()
{
	assert(p->phase == phaseReadContent);
	// Read a single line: the function will be called again if another line is ready
	std::string line;
	const std::size_t len = std::size_t(readLineFromStdout(line));
	if (len == 0)
	{
		if (p->contentLength > 0)
		{
			p->phase = phaseReadMessage;
			p->offset = 0;
		}
		return;
	}
	std::string_view mod(line);
	if (mod.find_first_of("Content-Length: ") == 0)
	{
		mod.remove_prefix(16);
		p->contentLength = std::size_t(std::atoll(mod.data()));
		p->content.resize(p->contentLength);
	}
}

void LspProcess::onProcessExited(int exitcode)
{
	Platform::DebugPrintf("LSP external process exited with code %d", exitcode);
	p->handler->onLspProcessExited();
}
