// Scintilla source code edit control
/** @file ProcessWin.cxx
 ** Class for launching a child process
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "process.h"

#include <Windows.h>
#include <processthreadsapi.h>

/*
TODO: 	
int Process::waitProcessFinished(int tout_ms = -1);

Implement an event handler that
 - calls outDataAvailable(int avail) when there is data to read on standard output
 - calls errDataAvailable(int avail) when there is data to read on standard error (defaulting to print data to debug output)
 - does monitor program exit, and print on debug output program exit code
 */

std::size_t Scintilla::Process::GetCurrentProcessId()
{
	return ::GetCurrentProcessId();
}

#include <windows.h> 
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include "UniConversion.h"

namespace Scintilla {

class ProcessPrivate
{
public:
	ProcessPrivate()
		: g_hChildStd_IN_Rd(nullptr),
		g_hChildStd_IN_Wr(nullptr),
		g_hChildStd_OUT_Rd(nullptr),
		g_hChildStd_OUT_Wr(nullptr),
		g_hChildStd_ERR_Rd(nullptr),
		g_hChildStd_ERR_Wr(nullptr)
	{
	}
	~ProcessPrivate() { closeHandles(); }
	void closeHandles();
	int createPipes();
	int createChildProcess();

	HANDLE g_hChildStd_IN_Rd;
	HANDLE g_hChildStd_IN_Wr;
	HANDLE g_hChildStd_OUT_Rd;
	HANDLE g_hChildStd_OUT_Wr;
	HANDLE g_hChildStd_ERR_Rd;
	HANDLE g_hChildStd_ERR_Wr;

	HANDLE hProcess;

	std::string executable;
	std::string args;
};

void ProcessPrivate::closeHandles()
{
	if (g_hChildStd_IN_Rd) CloseHandle(g_hChildStd_IN_Rd);
	if (g_hChildStd_IN_Wr) CloseHandle(g_hChildStd_IN_Wr);
	if (g_hChildStd_OUT_Rd) CloseHandle(g_hChildStd_OUT_Rd);
	if (g_hChildStd_OUT_Wr) CloseHandle(g_hChildStd_OUT_Wr);
	if (g_hChildStd_ERR_Rd) CloseHandle(g_hChildStd_ERR_Rd);
	if (g_hChildStd_ERR_Wr) CloseHandle(g_hChildStd_ERR_Wr);
	g_hChildStd_IN_Rd = g_hChildStd_IN_Wr = g_hChildStd_OUT_Rd = g_hChildStd_OUT_Wr = g_hChildStd_ERR_Rd = g_hChildStd_ERR_Wr = nullptr;
}

int ProcessPrivate::createPipes()
{
	// https://docs.microsoft.com/en-us/windows/win32/ProcThread/creating-a-child-process-with-redirected-input-and-output

	SECURITY_ATTRIBUTES saAttr;

// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = true;
	saAttr.lpSecurityDescriptor = nullptr;

// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		return GetLastError();

// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		return GetLastError();

// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		return GetLastError();

// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		return GetLastError();

// Create a pipe for the child process's STDERR. 

	if (!CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &saAttr, 0))
		return GetLastError();

// Ensure the read handle to the pipe for STDERR is not inherited.

	if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0))
		return GetLastError();

	return NO_ERROR;
}

//! Create a child process that uses the previously created pipes for STDIN and STDOUT.
int ProcessPrivate::createChildProcess()
{
	hProcess = {};

	const int cpe = createPipes();
	if (cpe != NO_ERROR)
	{
		closeHandles();
		return cpe;
	}

	// Convert strings
	// TODO: check that executable does not have spaces, or it is already a quoted string
	std::wstring _exe = WStringFromUTF8(std::string_view(executable)) + WStringFromUTF8(std::string_view(args));
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;

// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

// Set up members of the STARTUPINFO structure. 
// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_ERR_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

// Create the child process. 

	const bool bSuccess = CreateProcess(nullptr,
		_exe.data(),   // command line 
		nullptr,       // process security attributes 
		nullptr,       // primary thread security attributes 
		true,          // handles are inherited 
		0,             // creation flags 
		nullptr,       // use parent's environment 
		nullptr,       // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	hProcess = piProcInfo.hProcess;

	// If an error occurs, exit the application. 
	if (!bSuccess)
	{
		closeHandles();
		return GetLastError();
	}
	// Close handles to the child process and its primary thread.
	// Some applications might keep these handles to monitor the status
	// of the child process, for example. 
	//CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	return NO_ERROR;
}

constexpr DWORD pipeBufferSize = 64 * 1024;

Process::Process()
	: p(new ProcessPrivate)
{
}

Process::~Process()
{
	delete p;
}

void Process::setExecutable(const char *exec, const char *params)
{
	p->executable = exec;
	p->args = params;
}

int Process::start()
{
	return p->createChildProcess();
}

namespace
{

size_t availableHandle(HANDLE handle)
{
	DWORD bytesRead = 0;
	DWORD bytesAvail = 0;
	std::string buffer(pipeBufferSize, '\0');

	if (!::PeekNamedPipe(handle, buffer.data(),
		static_cast<DWORD>(buffer.size()), &bytesRead, &bytesAvail, NULL))
	{
		bytesAvail = 0;
	}

	return bytesRead + bytesAvail;
}

bool canReadLineHandle(HANDLE handle)
{
	DWORD bytesRead = 0;
	DWORD bytesAvail = 0;
	std::string buffer(pipeBufferSize, '\0');

	if (!::PeekNamedPipe(handle, buffer.data(),
		static_cast<DWORD>(buffer.size()), &bytesRead, &bytesAvail, NULL))
	{
		bytesAvail = 0;
	}

	buffer.erase(bytesRead);
	return buffer.find('\n') != std::string::npos;
}

size_t lineLengthHandle(HANDLE handle)
{
	DWORD bytesRead = 0;
	DWORD bytesAvail = 0;
	std::string buffer(pipeBufferSize, '\0');

	if (!::PeekNamedPipe(handle, buffer.data(),
		static_cast<DWORD>(buffer.size()), &bytesRead, &bytesAvail, NULL))
	{
		bytesAvail = 0;
	}

	buffer.erase(bytesRead);
	const size_t nl = buffer.find('\n');

	if (nl != std::string::npos)
	{
		return nl + 1;
	}
	else
	{
		return buffer.size();
	}
}

int readLineFromHandle(HANDLE handle, std::string &line)
{
	const size_t lineLength = lineLengthHandle(handle);
	line.resize(lineLength);
	DWORD dwRead = 0;
	if (!ReadFile(handle, line.data(), static_cast<DWORD>(lineLength), &dwRead, nullptr))
		return 0;
	while (line.size() > 0 && ((line.back() == '\r') || (line.back() == '\n')))
	{
		line.pop_back();
	}
	return int(line.size());
}

}

bool Process::canReadLineStdOut()
{
	return canReadLineHandle(p->g_hChildStd_OUT_Rd);
}

bool Process::canReadLineStdErr()
{
	return canReadLineHandle(p->g_hChildStd_ERR_Rd);
}

int Process::writeToStdin(const char *buf, std::size_t len)
{
	// Write to the pipe for the child's STDIN
	DWORD dwWritten;
	if (!WriteFile(p->g_hChildStd_IN_Wr, buf, static_cast<DWORD>(len), &dwWritten, nullptr))
		return 0;
	return dwWritten;
}

int Process::readFromStdout(char *buf, std::size_t maxLen)
{
	// Read from the child process's pipe for STDOUT
	DWORD dwRead;
	if (!ReadFile(p->g_hChildStd_OUT_Rd, buf, static_cast<DWORD>(maxLen), &dwRead, nullptr))
		return 0;
	return dwRead;
}

int Process::readLineFromStdout(std::string &line) {
	return readLineFromHandle(p->g_hChildStd_OUT_Rd, line);
}

int Process::readFromStderr(char *buf, std::size_t maxLen)
{
	// Read output from the child process's pipe for STDOUT
	DWORD dwRead;
	if (!ReadFile(p->g_hChildStd_ERR_Rd, buf, static_cast<DWORD>(maxLen), &dwRead, nullptr))
		return 0;
	return dwRead;
}

int Process::readLineFromStderr(std::string &line) {
	return readLineFromHandle(p->g_hChildStd_ERR_Rd, line);
}

void Process::outDataAvailable(std::size_t)
{
	while (canReadLineStdOut())
		outLineAvailable();
}

void Process::outLineAvailable()
{
	std::string out;
	if (readLineFromStdout(out) > 0)
	{
		OutputDebugStringA("OUT: ");
		OutputDebugStringA(out.data());
		OutputDebugStringA("\n");
	}
}

void Process::errDataAvailable(std::size_t)
{
	while (canReadLineStdErr())
		errLineAvailable();
}

void Process::errLineAvailable()
{
	std::string err;
	if (readLineFromStderr(err) > 0)
	{
		OutputDebugStringA("ERR: ");
		OutputDebugStringA(err.data());
		OutputDebugStringA("\n");
	}
}

int Process::waitProcessFinished(int tout_ms)
{
	if (WAIT_OBJECT_0 != ::WaitForSingleObject(p->hProcess, tout_ms)) {
		// We should use it only if the GUI process is stuck and
		// don't answer to a normal termination command.
		// This function is dangerous: dependent DLLs don't know the process
		// is terminated, and memory isn't released.
		OutputDebugStringA("\n>Process failed to respond; forcing abrupt termination...\n");
		::TerminateProcess(p->hProcess, 1);
	}

	return 0;
}

void Process::pollIO()
{
	const size_t availableOut = availableHandle(p->g_hChildStd_OUT_Rd);
	if (availableOut > 0)
	{
		outDataAvailable(availableOut);
	}

	const size_t availableErr = availableHandle(p->g_hChildStd_ERR_Rd);
	if (availableErr > 0)
	{
		errDataAvailable(availableErr);
	}

}

}	// namespace Scintilla
