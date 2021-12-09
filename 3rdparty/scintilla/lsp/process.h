// Scintilla source code edit control
/** @file process.h
 ** Class for launching a child process
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCINTILLA_PROCESS_H
#define SCINTILLA_PROCESS_H

#include <cstddef>	// std::size_t
#include <string>

namespace Scintilla {

class ProcessPrivate;

//! Class to handle starting a new process and communicate with it through stdin/stdout
class Process
{
public:
	//! Get process ID for the server
	static std::size_t GetCurrentProcessId();

public:
	Process();
	virtual ~Process();

	//! \brief Set the server executable path and parameters
	void setExecutable(const char *exec, const char *params);

	//! Start the process
	int start();
	//! Test if process started
	bool isStarted() const;

	//! Test if a complete line is ready on stdout
	bool canReadLineStdOut();
	//! Test if a complete line is ready on stdout
	bool canReadLineStdErr();
	//! Write data to the external process
	int writeToStdin(const char *buf, std::size_t len);
	//! Read data from the external process stdout
	int readFromStdout(char *buf, std::size_t maxLen);
	//! Read a single line from stdout
	int readLineFromStdout(std::string &line);
	//! Read data from the external process stderr
	int readFromStderr(char *buf, std::size_t maxLen);
	//! Read a single line from stderr
	int readLineFromStderr(std::string &line);
	//! Wait process finished for maximum time tout_ms
	int waitProcessFinished(int tout_ms = -1);

	void pollIO();

private:
	/*!
	 *	\brief Called when the server process has been started
	 */
	virtual void onProcessStarted() {}
	/*!
	 *	\brief Called when data is available from stdout of the child process
	 *	\details The default implementation calls \ref outLineAvailable()
	 *	as soon as there is a line ready to read
	 */
	virtual void outDataAvailable(std::size_t sz);
	//! Called when a line of data is available from stdout of the child process
	virtual void outLineAvailable();
	/*!
	 *	\brief Called when data is available from stderr of the child process
	 *	\details The default implementation calls \ref errLineAvailable()
	 *	as soon as there is a line ready to read
	 */
	virtual void errDataAvailable(std::size_t sz);
	//! Called when a line of data is available from stderr of the child process
	virtual void errLineAvailable();
	//! Function called after exit of the child process
	virtual void onProcessExited([[maybe_unused]] int exitcode) {}

private:
	//! Implementation details
	ProcessPrivate *p;
};

}

#endif	// SCINTILLA_PROCESS_H
