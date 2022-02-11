#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "dap/protocol.h"
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

namespace  dap {

// extend dap::LaunchRequest
class LaunchRequestExtend : public LaunchRequest
{
public:
    dap::string name;
    dap::string type;
    dap::string program;
};

DAP_STRUCT_TYPEINFO_EXT(LaunchRequestExtend,
                        LaunchRequest,
                        "launch",
                        DAP_FIELD(name, "name"),
                        DAP_FIELD(type, "type"),
                        DAP_FIELD(program, "program"));
} //namespace dap

namespace cxxdbg {

// return capabilities json object
QJsonObject getCapabilitiesJsonObj()
{
    QJsonObject capabilitiesJsonObj{};
    capabilitiesJsonObj.insert("supportsConfigurationDoneRequest", true);
    capabilitiesJsonObj.insert("supportsFunctionBreakpoints", true);
    capabilitiesJsonObj.insert("supportsConditionalBreakpoints", false);
    capabilitiesJsonObj.insert("supportsHitConditionalBreakpoints", false);
    capabilitiesJsonObj.insert("supportsEvaluateForHovers", false);
    capabilitiesJsonObj.insert("supportsBreakpointFilters", false);
    capabilitiesJsonObj.insert("supportsStepBack", false);
    capabilitiesJsonObj.insert("supportsSetVariable", false);
    capabilitiesJsonObj.insert("supportsRestartFrame", false);
    capabilitiesJsonObj.insert("supportsGotoTargetsRequest", false);
    capabilitiesJsonObj.insert("supportsStepInTargetsRequest", false);
    capabilitiesJsonObj.insert("supportsCompletionsRequest", false);
    capabilitiesJsonObj.insert("supportsModulesRequest", false);
    capabilitiesJsonObj.insert("supportsRestartReqeust", false);
    capabilitiesJsonObj.insert("supportsExceptionOptions", false);
    capabilitiesJsonObj.insert("supportsValueFormattingOptions", false);
    capabilitiesJsonObj.insert("supportsExceptionInfoRequest", false);
    capabilitiesJsonObj.insert("supportsTerminateDebuggee", false);
    capabilitiesJsonObj.insert("supportsSuspendDebuggee", false);
    capabilitiesJsonObj.insert("supportsDelayedStackTraceLoading", false);
    capabilitiesJsonObj.insert("supportsLoadedSourceRequest", false);
    capabilitiesJsonObj.insert("supportsLogPoints", false);
    capabilitiesJsonObj.insert("supportsTerminateThreadsRequest", false);
    capabilitiesJsonObj.insert("supportsSetExpression", false);
    capabilitiesJsonObj.insert("supportsTerminateRequest", false);
    capabilitiesJsonObj.insert("supportsDataBreakpoints", false);
    capabilitiesJsonObj.insert("supportsReadMemoryRequest", false);
    capabilitiesJsonObj.insert("supportsWriteMemoryRequest", false);
    capabilitiesJsonObj.insert("supportsDisassembleRequest", false);
    capabilitiesJsonObj.insert("supportsCancelRequest", false);
    capabilitiesJsonObj.insert("supportsBreakpointLocationsRequest", false);
    capabilitiesJsonObj.insert("supportsClipboardContext", false);
    capabilitiesJsonObj.insert("supportsSteppingGranularity", false);
    capabilitiesJsonObj.insert("supportsInstructionBreakpoints", false);
    capabilitiesJsonObj.insert("supportsExceptionFilterOptions", false);
    capabilitiesJsonObj.insert("supportsSingleThreadExecutionRequests", false);
    return capabilitiesJsonObj;
}

// return initialize Json Object
QJsonObject getInitializeJsonObj()
{
    QJsonObject initializeJsonObj{
        {"clientID", "clientID"},
        {"clientname", "clientName"},
        {"adapterID", "adpaterID"},
        {"locale", "en-US"},
        {"linesStartAt1", true},
        {"columnsStartAt1", true},
        {"pathFormat", "path"},
        {"supportsVariableType", false},
        {"supportsRunInTerminalRequest", false},
        {"supportsMemoryReferences", false},
        {"supportsProgressReporting", false},
        {"supportsInvalidatedEvent", false},
        {"supportsMemoryEvent", false}
    };
    return initializeJsonObj;
}

// return launch json object
QJsonObject getLaunchJsonObj()
{
    QJsonObject launchJsonObj{};
    launchJsonObj.insert("name", "name");
    launchJsonObj.insert("type", "type");
    launchJsonObj.insert("program", "program");
    return launchJsonObj;
}

QJsonObject getAttachJsonObj()
{
    QJsonObject attachJsonObj{};
    return attachJsonObj;
}

QJsonObject getInitializedJsonObj()
{
    QJsonObject initialized{};
    return initialized;
}

// get configuration done json object
QJsonObject getConfigurationDoneJsonObj()
{
    QJsonObject configurationDone{};
    return configurationDone;
}
} // namespace cxxdbg
#endif // PROTOCOL_H
