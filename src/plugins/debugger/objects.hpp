/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <QObject>

namespace objects {

/**
 * Copies all properties of source into destination. The optional parameter "overwrite" allows to control
 * if existing properties on the destination should be overwritten or not. Defaults to true (overwrite).
 */
template <class T, class K>
T mixin(T destination, K source, bool overwrite = true)
{
    Q_UNUSED(source)
    Q_UNUSED(overwrite)
    // TODO(mozart):Modify according to the comment.
    // The set of additional module information exposed by the debug adapter.
    destination.additionalModuleColumns = source.additionalModuleColumns;
    // The set of characters that should trigger completion in a REPL. If not
    // specified, the UI should assume the '.' character.
    destination.completionTriggerCharacters = source.completionTriggerCharacters;
    // Available exception filter options for the 'setExceptionBreakpoints'
    // request.
    destination.exceptionBreakpointFilters = source.exceptionBreakpointFilters;
    // The debug adapter supports the 'terminateDebuggee' attribute on the
    // 'disconnect' request.
    destination.supportTerminateDebuggee = source.supportTerminateDebuggee;
    // Checksum algorithms supported by the debug adapter.
    destination.supportedChecksumAlgorithms = source.supportedChecksumAlgorithms;
    // The debug adapter supports the 'breakpointLocations' request.
    destination.supportsBreakpointLocationsRequest = source.supportsBreakpointLocationsRequest;
    // The debug adapter supports the 'cancel' request.
    destination.supportsCancelRequest = source.supportsCancelRequest;
    // The debug adapter supports the 'clipboard' context value in the 'evaluate'
    // request.
   destination.supportsClipboardContext = source.supportsClipboardContext;
    // The debug adapter supports the 'completions' request.
    destination.supportsCompletionsRequest = source.supportsClipboardContext;
    // The debug adapter supports conditional breakpoints.
    destination.supportsConditionalBreakpoints = source.supportsConditionalBreakpoints;
    // The debug adapter supports the 'configurationDone' request.
    destination.supportsConfigurationDoneRequest = source.supportsConfigurationDoneRequest;
    // The debug adapter supports data breakpoints.
    destination.supportsDataBreakpoints = source.supportsDataBreakpoints;
    // The debug adapter supports the delayed loading of parts of the stack, which
    // requires that both the 'startFrame' and 'levels' arguments and an optional
    // 'totalFrames' result of the 'StackTrace' request are supported.
    destination.supportsDelayedStackTraceLoading = source.supportsDelayedStackTraceLoading;
    // The debug adapter supports the 'disassemble' request.
    destination.supportsDisassembleRequest = source.supportsDisassembleRequest;
    // The debug adapter supports a (side effect free) evaluate request for data
    // hovers.
    destination.supportsEvaluateForHovers = source.supportsEvaluateForHovers;
    // The debug adapter supports 'filterOptions' as an argument on the
    // 'setExceptionBreakpoints' request.
    destination.supportsExceptionFilterOptions = source.supportsExceptionFilterOptions;
    // The debug adapter supports the 'exceptionInfo' request.
    destination.supportsExceptionInfoRequest = source.supportsExceptionInfoRequest;
    // The debug adapter supports 'exceptionOptions' on the
    // setExceptionBreakpoints request.
    destination.supportsExceptionOptions = source.supportsExceptionOptions;
    // The debug adapter supports function breakpoints.
    destination.supportsFunctionBreakpoints = source.supportsFunctionBreakpoints;
    // The debug adapter supports the 'gotoTargets' request.
    destination.supportsGotoTargetsRequest = source.supportsGotoTargetsRequest;
    // The debug adapter supports breakpoints that break execution after a
    // specified number of hits.
    destination.supportsHitConditionalBreakpoints = source.supportsHitConditionalBreakpoints;
    // The debug adapter supports adding breakpoints based on instruction
    // references.
    destination.supportsInstructionBreakpoints = source.supportsInstructionBreakpoints;
    // The debug adapter supports the 'loadedSources' request.
    destination.supportsLoadedSourcesRequest = source.supportsLoadedSourcesRequest;
    // The debug adapter supports logpoints by interpreting the 'logMessage'
    // attribute of the SourceBreakpoint.
    destination.supportsLogPoints = source.supportsLogPoints;
    // The debug adapter supports the 'modules' request.
    destination.supportsModulesRequest = source.supportsModulesRequest;
    // The debug adapter supports the 'readMemory' request.
    destination.supportsReadMemoryRequest = source.supportsReadMemoryRequest;
    // The debug adapter supports restarting a frame.
    destination.supportsRestartFrame = source.supportsRestartFrame;
    // The debug adapter supports the 'restart' request. In this case a client
    // should not implement 'restart' by terminating and relaunching the adapter
    // but by calling the RestartRequest.
    destination.supportsRestartRequest = source.supportsRestartRequest;
    // The debug adapter supports the 'setExpression' request.
    destination.supportsSetExpression = source.supportsSetExpression;
    // The debug adapter supports setting a variable to a value.
    destination.supportsSetVariable = source.supportsSetVariable;
    // The debug adapter supports stepping back via the 'stepBack' and
    // 'reverseContinue' requests.
    destination.supportsStepBack = source.supportsStepBack;
    // The debug adapter supports the 'stepInTargets' request.
    destination.supportsStepInTargetsRequest = source.supportsStepInTargetsRequest;
    // The debug adapter supports stepping granularities (argument 'granularity')
    // for the stepping requests.
    destination.supportsSteppingGranularity = source.supportsSteppingGranularity;
    // The debug adapter supports the 'terminate' request.
    destination.supportsTerminateRequest = source.supportsTerminateRequest;
    // The debug adapter supports the 'terminateThreads' request.
    destination.supportsTerminateThreadsRequest = source.supportsTerminateThreadsRequest;
    // The debug adapter supports a 'format' attribute on the stackTraceRequest,
    // variablesRequest, and evaluateRequest.
    destination.supportsValueFormattingOptions = source.supportsValueFormattingOptions;

    return destination;
}

}

#endif // OBJECTS_HPP
