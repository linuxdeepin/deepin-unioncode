// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tools.h"

#include <QDebug>
#include <QDir>
#include <QMetaEnum>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QApplication>

#include <iostream>
#include <vector>

namespace {
// perf top tool
static std::string overhead{"overhead"};
static std::string sample{"sample"};
static std::string period{"period"};
static std::string dso{"dso"};
static std::string symbol{"symbol"};
static std::string srcline{"srcline"};

/* top tool command out VIRT field :
 * VIRT stands for the virtual size of a process,
 * which is the sum of memory it is actually using,
 * memory it has mapped into itself (for instance the video card’s RAM for the X server),
 * files on disk that have been mapped into it (most notably shared libraries),
 * and memory shared with other processes.
 * VIRT represents how much memory the program is able to access at the present moment.
 */

/* top tool command out RES field :
 * RES stands for the resident size,
 * which is an accurate representation of how much actual physical memory a process is consuming.
 * (This also corresponds directly to the %MEM column.) This will virtually always be less than the VIRT size,
 * since most programs depend on the C library.
 */

/* top tool command out SHR field :
 * SHR indicates how much of the VIRT size is actually sharable (memory or libraries).
 * In the case of libraries, it does not necessarily mean that the entire library is resident.
 * For example, if a program only uses a few functions in a library,
 * the whole library is mapped and will be counted in VIRT and SHR,
 * but only the parts of the library file containing the functions being used will actually be loaded in and be counted under RES.
 */
static std::string processId{"processId"};
static std::string user{"user"};
static std::string priority{"priority"};
static std::string nice{"nice"};
static std::string virtualSize{"virtualSize"};
static std::string residentSize{"residentSize"};
static std::string sharable{"sharable"};
static std::string status{"status"};
static std::string cpuOverhead{"cpuOverhead"};
static std::string memoryOverhead{"memoryOverhead"};
static std::string timeCount{"timeCount"};
static std::string command{"command"};

static std::string fd{"fd"};
static std::string type{"type"};
static std::string device{"device"};
static std::string sizeOff{"sizeOff"};
static std::string node{"node"};
static std::string name{"name"};

static std::string netId{"netId"};
static std::string state{"state"};
static std::string recvQueue{"recvQueue"};
static std::string sendQueue{"sendQueue"};
static std::string localAddrPort{"localAddrPort"};
static std::string peerAddrPort{"peerAddressPort"};
static std::string usingProcesses{"usingProcesses"};

// device IO field
static std::string userId{"userId"};
//static std::string processId{"processId"};
static std::string kB_rd{"kB_rd"};
static std::string kB_wd{"kB_wd"};
static std::string kB_ccwr{"kB_ccwr"};
static std::string iodelay{"iodelay"};
//static std::string command{"command"};


static std::string feild{"feild"};
static std::string key{"key"};
static std::string lines{"lines"};

static std::string package_name{"package_name"};
static std::string system_default{"system default"};

template<class E>
const char * enumName()
{
    return QMetaEnum::fromType<E>().name();
}

template<class Elem>
const char * enumElemName(Elem type)
{
    return QMetaEnum::fromType<Elem>().valueToKey(type);
}

struct QStr : QString
{
    QStr(const std::string &src) : QString(QString::fromStdString(src)){}
};

struct Add : QString, std::string
{
    Add(const QString &x, const QString &y)
        : QString(QString::number(x.toULongLong() + y.toULongLong()))
        , std::string(QString::toStdString())
    {

    }

    Add(const std::string &x, const std::string &y)
        : QString(QString::number(QStr(x).toLongLong() + QStr(y).toLongLong()))
        , std::string(QString::toStdString())
    {

    }
};

QString format(const QString &src)
{
    QString temp = src;
    temp.replace(QRegularExpression("[\\s]+"), " ");
    while (temp.startsWith(" ")) {
        temp.remove(0 , sizeof(" "));
    }
    while (temp.endsWith(" ")) {
        temp.remove(temp.size() - 1, sizeof(" "));
    }
    return temp;
};

struct Template
{
    static QString scriptsPath()
    {
        QString cacheChildPath = QCoreApplication::applicationName();
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        dir.cdUp();
        if (!dir.cd(cacheChildPath)) {
            dir.mkdir(cacheChildPath);
            dir.cd(cacheChildPath);
        }

        qInfo() << dir.path();
        QString scriptChildPath = "scripts";
        if (!dir.cd(scriptChildPath)) {
            dir.mkdir(scriptChildPath);
            dir.cd(scriptChildPath);
        }
        qInfo() << dir.path();
        return dir.path();
    }

    static QString scriptsPathFrom(Tools::PerformanceType pt)
    {
        return scriptsPath() + QDir::separator() + enumElemName<Tools::PerformanceType>(pt) + "Lanuch.sh";
    }

    static void checkSocketScript()
    {
        QString filePath = scriptsPathFrom(Tools::PerformanceType::socket);
        QFile file(filePath);
        QByteArray readData;
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            readData = file.readAll();
            file.close();
        } else {
            qCritical() << "can't open socket script" << filePath;
        }

        if (readData.isEmpty() || socketScriptCode() != readData) {
            QFile file(filePath);
            if (file.open(QFile::Truncate | QFile::WriteOnly | QFile::Text)) {
                file.write(socketScriptCode());
                file.setPermissions(QFile::Permission::ExeUser |
                                    QFile::Permission::ReadUser|
                                    QFile::Permission::WriteUser);
                file.close();
            }
        }
    }

    static QByteArray socketScriptCode(){
#ifdef __linux__
        return "#!/bin/bash\n"
               "if [ -z $1 ]; then\n"
               "    echo \"can't attach pid null\">&2\n"
               "    exit -1\n"
               "fi\n"
               "\n"
               "if [ -z $2 ]; then\n"
               "    echo \"can't parent pid null\">&2\n"
               "    exit -2\n"
               "fi\n"
               "\n"
               "while [ true ];\n"
               "do\n"
               "    if ps -p $1 >/dev/null; then\n"
               "        if ps -p $2 >/dev/null; then\n"
               "            ss -p|grep pid=$1;\n"
               "        else\n"
               "            echo \"can't found parent pid:'$2', add watch from parent\">&2\n"
               "        fi\n"
               "    else\n"
               "        echo \"can't found attach pid:'$1', process state no running\">&2\n"
               "        exit -1\n"
               "    fi\n"
               "    sleep 1;\n"
               "done\n";
#else
#endif
    }
};

struct JsonOp
{

    static void setField(const std::vector<std::string> &feild, Json::Value &jsonObj)
    {
        Json::Value feildArray(Json::arrayValue);
        for (auto && val : feild) {
            feildArray.append(Json::Value(val));
        }
        jsonObj[::feild] = feildArray;
    }


    static void setKey(const std::string &key, Json::Value &jsonObj)
    {
        jsonObj[::key] = key;
    }
};

}

class ToolsPrivate
{
    friend class Tools;
    QHash<Tools::PerformanceType, QProcess*> toolsIns{};
    QHash<Tools::PerformanceType, Json::Value> toolsCountCache{};
    int attachPID{0};
    QTextBrowser *textBrowser{nullptr};

    ~ToolsPrivate()
    {
        qDeleteAll(toolsIns);
        toolsCountCache.clear();
    }

    QTextBrowser *displayErrorWidget()
    {
        if (!textBrowser) {
            textBrowser = new QTextBrowser();
            textBrowser->setWindowFlag(Qt::Dialog);
            textBrowser->setWindowTitle(QCoreApplication::applicationName() + " warning");
            textBrowser->setMinimumSize(600,300);
            textBrowser->hide();
            qApp->setQuitOnLastWindowClosed(false);
        }
        return textBrowser;
    }

    void countFunction(const Json::Value &functionObj)
    {
        // no cache
        if (!toolsCountCache.contains(Tools::PerformanceType::function)) {
            toolsCountCache[Tools::PerformanceType::function] = functionObj;
        } else { // has cache
            // find array with cache
            Json::Value cache = toolsCountCache[Tools::PerformanceType::function];
            Json::Value cacheFeild = cache[::feild];
            Json::Value cacheLines = cache[::lines];
            Json::Value paramFeild = functionObj[::feild];
            Json::Value paramLines = functionObj[::lines];
            if (paramFeild.size() != cacheFeild.size()
                    && !cacheFeild.empty()) {
                qCritical() << __FUNCTION__ << "error feild not's same: "
                            << "cache: " << QString::fromStdString(cacheFeild.toStyledString())
                            << "current: " << QString::fromStdString(paramFeild.toStyledString());

                return;
            }

            int paramOverheadIdx = -1;
            int paramSampleIdx = -1;
            int paramPeriodIdx = -1;
            int paramDsoIdx = -1;
            int paramSymbolIdx = -1;
            int paramSrclineIdx = -1;
            int cacheOverheadIdx = -1;
            int cacheSampleIdx = -1;
            int cachePeriodIdx = -1;
            int cacheDsoIdx = -1;
            int cacheSymbolIdx = -1;
            int cacheSrclineIdx = -1;

            for (unsigned int idx = 0; idx < qMin(cacheFeild.size(), paramFeild.size()); idx ++) {
                if (cacheFeild[idx] == ::overhead) {
                    cacheOverheadIdx = static_cast<int>(idx);
                }
                if (cacheFeild[idx] == ::sample) {
                    cacheSampleIdx = static_cast<int>(idx);
                }
                if (cacheFeild[idx] == ::period) {
                    cachePeriodIdx = static_cast<int>(idx);
                }
                if (cacheFeild[idx] == ::dso) {
                    cacheDsoIdx = static_cast<int>(idx);
                }
                if (cacheFeild[idx] == ::symbol) {
                    cacheSymbolIdx = static_cast<int>(idx);
                }
                if (cacheFeild[idx] == ::srcline) {
                    cacheSrclineIdx = static_cast<int>(idx);
                }
                if (paramFeild[idx] == ::overhead) {
                    paramOverheadIdx = static_cast<int>(idx);
                }
                if (paramFeild[idx] == ::sample) {
                    paramSampleIdx = static_cast<int>(idx);
                }
                if (paramFeild[idx] == ::period) {
                    paramPeriodIdx = static_cast<int>(idx);
                }
                if (paramFeild[idx] == ::dso) {
                    paramDsoIdx = static_cast<int>(idx);
                }
                if (paramFeild[idx] == ::symbol) {
                    paramSymbolIdx = static_cast<int>(idx);
                }
                if (paramFeild[idx] == ::srcline) {
                    paramSrclineIdx = static_cast<int>(idx);
                }
            }

            for (unsigned int paramRow = 0; paramRow < paramLines.size(); paramRow ++) {
                auto paramLine = paramLines[paramRow];
                bool hasSame = false;
                for (unsigned int cacheRow = 0; cacheRow < cacheLines.size();  cacheRow ++) {
                    auto cacheLine = cacheLines[cacheRow];
                    if (paramSymbolIdx > 0 && cacheSymbolIdx > 0) {
                        if (paramLine[paramSymbolIdx].asString() == cacheLine[cacheSymbolIdx].asString()) {
                            if (cacheOverheadIdx > 0 && paramOverheadIdx > 0)
                                cacheLine[cacheOverheadIdx] = paramLine[paramOverheadIdx].asString();
                            if (cacheSampleIdx > 0 && cacheSampleIdx > 0)
                                cacheLine[cacheSampleIdx] = Add(cacheLine[cacheSampleIdx].asString(), paramLine[paramSampleIdx].asString());
                            if (cachePeriodIdx > 0 && paramPeriodIdx > 0)
                                cacheLine[cachePeriodIdx] = Add(cacheLine[cachePeriodIdx].asString(), paramLine[paramPeriodIdx].asString());
                            if (cacheDsoIdx > 0 && paramDsoIdx > 0)
                                cacheLine[cacheDsoIdx] = paramLine[paramDsoIdx].asString();
                            if (cacheSymbolIdx > 0 && paramSymbolIdx > 0)
                                cacheLine[cacheSymbolIdx] = paramLine[paramSymbolIdx].asString();
                            if (cacheSrclineIdx > 0 && paramSrclineIdx > 0)
                                cacheLine[cacheSrclineIdx] = paramLine[paramSrclineIdx].asString();
                            cacheLines[cacheRow] = cacheLine;
                            hasSame = true;
                            break;
                        }
                    }
                }
                if (!hasSame) {
                    cacheLines.append(paramLine);
                }
            }

            // rewrite overhead
            long long countPeriod = 0;
            for(unsigned int idx = 0; idx < cacheLines.size(); idx ++) {
                Json::Value cacheLine = cacheLines[idx];
                if (!cacheLine.empty()) {
                    countPeriod += QStr(cacheLine[cachePeriodIdx].asString()).toLongLong();
                }
            }
            if (countPeriod > 0) {
                for (unsigned int i = 0; i < cacheLines.size(); i++) {
                    Json::Value cacheLine =  cacheLines[i];
                    if (!cacheLine.empty()) {
                        long long funcOnePeriod = QStr(cacheLine[cachePeriodIdx].asString()).toLongLong();
                        double onePeriod = double(funcOnePeriod)/ double(countPeriod);
                        // qInfo() << "funcOnePeriod" << funcOnePeriod << "countPeriod" << countPeriod;
                        cacheLine[cacheOverheadIdx] = QString(QString::number(onePeriod * 100, 10, 2)+ "%").toStdString();
                        // qInfo() << QStr(funcOne[::overhead].asString());
                        // qInfo() << "\n";
                    }
                    cacheLines[i] = cacheLine;
                }
            }
            cache[::lines] = cacheLines;
            toolsCountCache[Tools::PerformanceType::function] = cache;
        }
    }
};

Tools::Tools()
    : d(new ToolsPrivate)
{

}

Tools::~Tools()
{
    stopAll();
    if (d) {
        if (d->textBrowser) {
            delete d->textBrowser;
        }
        delete d;
    }
}

void Tools::startAll()
{
    for (auto ins : d->toolsIns) {
        ins->start();
        qInfo() << "start" << ins->program() << ins->arguments()
                << "pid :" << ins->processId();
    }
}

void Tools::stopAll()
{
    std::cout <<  __FUNCTION__ << std::endl;
    for (auto ins : d->toolsIns) {
        ins->kill();
        ins->waitForFinished();
    }
    d->toolsCountCache.clear();
}

void Tools::setAttachPID(int pid)
{
    std::cout << __FUNCTION__ << ": " <<pid << std::endl;
    d->attachPID = pid;

    if (d->displayErrorWidget()) {
        d->displayErrorWidget()->clear();
    }

    initTool(function);
    initTool(global);
    initTool(vfs);
    initTool(socket);
    initTool(deviceIO);
}

int Tools::attachPID() const
{
    return d->attachPID;
}

Json::Value Tools::data() const
{
    Json::Value result;
    for (auto key : d->toolsCountCache.keys()) {
        auto value = d->toolsCountCache.value(key);
        if (!value.empty()) {
            result[enumElemName(key)] = value;
        }
    }
    return result;
}

bool Tools::initTool(Tools::PerformanceType pt)
{
    std::cout << __FUNCTION__ << " : "
              << enumElemName<PerformanceType>(pt)
              << std::endl;

    if (d->attachPID <= 0) {
        qCritical() << "unknown process from attach PID" << d->attachPID;
    }

    QProcess* process = new QProcess;
    process->setEnvironment({"LINES=1000", "COLUMNS=1000"});
    if (PerformanceType::function == pt) {
        process->setProperty(package_name.c_str(), "perf-tools-unstable");
        process->setProperty(::enumName<PerformanceType>(), function);
        process->setProgram("/usr/bin/perf");
        process->setArguments({"top",
                               "--delay", "1",
                               "--fields","overhead,sample,period,dso,symbol,srcline",
                               "--hide_kernel_symbols",
                               "--verbose",
                               "--stdio",
                               "-p", QString::number(d->attachPID)});
        QDir tempDir(QDir::tempPath());
        if (!tempDir.exists("perf_top_redirect_stdout")) {
            qInfo()  << "create redirect output from perf top";
            QFile file(tempDir.filePath("perf_top_redirect_stdout"));
            file.open(QFile::OpenModeFlag::NewOnly);
            file.close();
        }
        process->setStandardInputFile(tempDir.filePath("perf_top_redirect_stdout"));
    }
    if (PerformanceType::global == pt) {
        process->setProperty(package_name.c_str(), system_default.c_str());
        process->setProperty(::enumName<PerformanceType>(), global);
        process->setProgram("/usr/bin/top");
        process->setArguments({"-b",
                               "-d","1",
                               "-p", QString::number(d->attachPID)});
    }

    if (PerformanceType::vfs == pt) {
        process->setProperty(package_name.c_str(), system_default.c_str());
        process->setProperty(::enumName<PerformanceType>(), vfs);
        process->setProgram("/usr/bin/lsof");
        process->setArguments({"-r", "1",
                               "-p", QString::number(d->attachPID)});
    }

    if (PerformanceType::socket == pt) {
        Template::checkSocketScript();
        process->setProperty(package_name.c_str(), system_default.c_str());
        process->setProperty(::enumName<PerformanceType>(), socket);
        process->setProgram(Template::scriptsPathFrom(socket));
        process->setArguments({QString::number(d->attachPID), QString::number(QCoreApplication::applicationPid())});
    }

    if (PerformanceType::deviceIO == pt) {
        process->setProperty(package_name.c_str(), "sysstat");
        process->setProperty(::enumName<PerformanceType>(), deviceIO);
        process->setProgram("/usr/bin/pidstat");
        process->setArguments({"-d", "1",
                               "-p", QString::number(d->attachPID)});
    }

    QObject::connect(process, &QProcess::readyReadStandardOutput,
                     this, &Tools::readAllStandardOutput,
                     Qt::UniqueConnection);

    QObject::connect(process, &QProcess::readyReadStandardError,
                     this, &Tools::readAllStandardError,
                     Qt::UniqueConnection);

    QObject::connect(process, &QProcess::errorOccurred,
                     this, &Tools::errorOccurred,
                     Qt::UniqueConnection);

    d->toolsIns.insert(pt, process);

    if (!process->program().isEmpty()) {
        return true;
    }

    return false;
}

QRegularExpression Tools::lineRegExpRule(Tools::PerformanceType pt)
{
    if (PerformanceType::global == pt)
        return QRegularExpression(
                    QString("\\s?") +
                    "(?<"+ ::QStr(::processId) + ">[0-9]+)\\s+" +
                    "(?<"+ QStr(::user) + ">\\S+)\\s+" +
                    "(?<"+ QStr(::priority) + ">[0-9][0-9])\\s+" +
                    "(?<"+ QStr(::nice) + ">-?[0-9]+)\\s+" +
                    "(?<"+ QStr(::virtualSize) + ">[0-9]+\\.?[0-9]?+\\S?)\\s+" +
                    "(?<"+ QStr(::residentSize) + ">[0-9]+\\.?[0-9]?+\\S?)\\s+" +
                    "(?<"+ QStr(::sharable) + ">[0-9]+\\.?[0-9]?+\\S?)\\s+" +
                    "(?<"+ QStr(::status) + ">[A-Za-z])\\s+" +
                    "(?<"+ QStr(::cpuOverhead) + ">[0-9]{1,2}.[0-9]+)\\s+" +
                    "(?<"+ QStr(::memoryOverhead) + ">[0-9]{1,2}.[0-9]+)\\s+" +
                    "(?<"+ QStr(::timeCount) + ">[0-9]+:[0-9]+.[0-9]+)\\s+" +
                    "(?<"+ QStr(::command) + ">\\S+)");
    if (PerformanceType::function == pt)
        return QRegularExpression(
                    QString("\\s+") +
                    "(?<" + QStr(::overhead) + ">[0-9]+.[0-9]+%)\\s+" +
                    "(?<" + QStr(::sample) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::period) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::dso) + ">[\\S]+)\\s+" +
                    "(?<" + QStr(::symbol) + ">\\S+\\s+\\w\\s+\\S+\\s[_:\\w<>(),&@*\\s]+)\\s+" +
                    "(?<" + QStr(::srcline) + ">\\S+)"
                    );
    if (PerformanceType::vfs == pt)
        return QRegularExpression(
                    QString("\\s?") +
                    "(?<" + QStr(::command) + ">\\S+)\\s+" +
                    "(?<" + QStr(::processId) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::user) + ">\\S+)\\s+" +
                    "(?<" + QStr(::fd) + ">[a-z0-9]+)\\s+" +
                    "(?<" + QStr(::type) + ">[a-zA-z_]+)\\s+" +
                    "(?<" + QStr(::device) + ">[0-9]+\\,[0-9]+)\\s+" +
                    "(?<" + QStr(::sizeOff) + ">[0-9a-zA-Z]+)\\s+" +
                    "(?<" + QStr(::node) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::name) + ">\\S+)\\s?"
                    );
    if (PerformanceType::socket == pt)
        return QRegularExpression(
                    QString("\\s?") +
                    "(?<" + QStr(::netId) + ">\\S+)\\s+" +
                    "(?<" + QStr(::state) + ">\\S+)\\s+" +
                    "(?<" + QStr(::recvQueue) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::sendQueue) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::localAddrPort) + ">[\\S]+\\s?:?[\\S]+)\\s+" +
                    "(?<" + QStr(::peerAddrPort) + ">[\\S]+\\s?:?[\\S]+)\\s+" +
                    "(?<" + QStr(::usingProcesses) + ">users:\\(\\S+\\))?"
                    );

    if (PerformanceType::deviceIO == pt)
        return QRegularExpression(
                    QString("\\s?") +
                    "(?<" + QStr(::userId) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::processId) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::kB_rd) + ">[0-9]+.[0-9]+)\\s+" +
                    "(?<" + QStr(::kB_wd) + ">[0-9]+.[0-9]+)\\s+" +
                    "(?<" + QStr(::kB_ccwr) + ">[0-9]+.[0-9]+)\\s+" +
                    "(?<" + QStr(::iodelay) + ">[0-9]+)\\s+" +
                    "(?<" + QStr(::command) + ">\\S+)\\s?"
                    );

    return QRegularExpression();
}

void Tools::readAllStandardOutput()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (process) {
        QVariant variant = process->property(::enumName<PerformanceType>());
        if (!variant.isValid()) {
            return;
        }
        if (variant.canConvert<PerformanceType>()) {
            PerformanceType type = qvariant_cast<PerformanceType>(variant);
            QString typeString = enumElemName<PerformanceType>(type);
            QRegularExpression regExp = lineRegExpRule(type);

            Json::Value globalObj(Json::objectValue);
            JsonOp::setField({::processId, ::user, ::priority, ::nice,
                              ::virtualSize, ::residentSize, ::sharable, status,
                              ::cpuOverhead, ::memoryOverhead, ::timeCount, ::command}
                             , globalObj);

            Json::Value functionObj(Json::objectValue);
            JsonOp::setField({::overhead, ::sample, ::period,
                              ::dso, ::symbol, srcline}
                             , functionObj);
            Json::Value functionLines = functionObj[::lines];

            Json::Value vfsObj(Json::objectValue);
            JsonOp::setField({::command, ::processId, ::user, ::fd,
                              ::type, ::device, ::sizeOff, ::node, ::name}
                             , vfsObj);

            Json::Value socketObj(Json::objectValue);
            JsonOp::setField({::netId, ::state, ::recvQueue, ::sendQueue,
                              ::localAddrPort, ::peerAddrPort, usingProcesses}
                             , socketObj);

            Json::Value deviceIOObj(Json::objectValue);
            JsonOp::setField({::userId, ::processId, ::kB_rd+"/s", ::kB_wd + "/s",
                              ::kB_ccwr + "/s", ::iodelay, ::command}
                             , deviceIOObj);

            while (process->canReadLine()) {
                auto lineData = process->readLine();
                if (!lineData.isEmpty()) {
                    auto matchs = regExp.match(lineData);
                    if (matchs.hasMatch()) {

                        if (function == type) {

                            Json::Value functionFeild = functionObj[::feild];
                            if (!functionFeild.empty() && functionFeild.isArray()) {

                                auto overheadData = ::format(matchs.captured(QStr(::overhead)));
                                auto sampleData = ::format(matchs.captured(QStr(::sample)));
                                auto periodData = ::format(matchs.captured(QStr(::period)));
                                auto dsoData = ::format(matchs.captured(QStr(::dso)));
                                auto symbolData = ::format(matchs.captured(QStr(::symbol)));
                                auto srclineData = ::format(matchs.captured(QStr(::srcline)));
                                if (!overheadData.isEmpty() && !sampleData.isEmpty()
                                        && !periodData.isEmpty() && !dsoData.isEmpty()
                                        && !symbolData.isEmpty() && !srclineData.isEmpty()) {

                                    Json::Value functionLine(Json::arrayValue);
                                    functionLine.resize(functionFeild.size());
                                    for (unsigned int feildIdx = 0; feildIdx < functionFeild.size(); feildIdx ++) {
                                        auto feild = functionFeild[feildIdx].asString();
                                        if (feild == ::overhead) {
                                            functionLine[feildIdx] = overheadData.toStdString();
                                        } else if (feild == ::sample) {
                                            functionLine[feildIdx] = sampleData.toStdString();
                                        } else if (feild == ::period) {
                                            functionLine[feildIdx] = periodData.toStdString();
                                        } else if (feild == ::dso) {
                                            functionLine[feildIdx] = dsoData.toStdString();
                                        } else if (feild == ::symbol) {
                                            functionLine[feildIdx] = symbolData.toStdString();
                                        } else if (feild == ::srcline) {
                                            functionLine[feildIdx] = srclineData.toStdString();
                                        } else {
                                            qCritical() << "unknown feild from performance type : function";
                                        }
                                    }

                                    int symbolIdx = -1;
                                    int sampleIdx = -1;
                                    int periodIdx = -1;
                                    for (unsigned int idx = 0; idx < functionFeild.size(); idx++) {
                                        if (functionFeild[idx].asString() == ::symbol) {
                                            symbolIdx = static_cast<int>(idx);
                                        } else if (functionFeild[idx].asString() == ::sample) {
                                            sampleIdx = static_cast<int>(idx);
                                        } else if (functionFeild[idx].asString() == ::period) {
                                            periodIdx = static_cast<int>(idx);
                                        } else {
                                            // noting to do
                                        }
                                    }


                                    bool hasSame = false;
                                    if (symbolIdx >= 0) {
                                        for (unsigned int lineIdx = 0; lineIdx < functionLines.size(); lineIdx ++) {
                                            auto beforLine = functionLines[lineIdx];
                                            if (!beforLine.empty() && beforLine[symbolIdx].asString() == symbolData.toStdString()) {
                                                functionLine[sampleIdx] = Add(functionLine[sampleIdx].asString(), beforLine[sampleIdx].asString());
                                                functionLine[periodIdx] = Add(functionLine[periodIdx].asString(), beforLine[periodIdx].asString());
                                                functionLines[lineIdx] = functionLine;
                                                hasSame = true;
                                                break;
                                            }
                                        }
                                    }

                                    if (!hasSame || functionLines.empty()) {
                                        functionLines.append(functionLine);
                                    }

                                    functionObj[::lines] = functionLines;
                                }
                            }
                        }

                        if (global == type) {
                            Json::Value feildArray = globalObj[::feild];
                            if (!feildArray.empty() && feildArray.isArray()) {
                                auto processIdData = ::format(matchs.captured(QStr(::processId)));
                                auto userData = ::format(matchs.captured(QStr(::user)));
                                auto priorityData = ::format(matchs.captured(QStr(::priority)));
                                auto niceData = ::format(matchs.captured(QStr(::nice)));
                                auto virtualSizeData = ::format(matchs.captured(QStr(::virtualSize)));
                                auto residentSizeData = ::format(matchs.captured(QStr(::residentSize)));
                                auto sharableData = ::format(matchs.captured(QStr(::sharable)));
                                auto statusData = ::format(matchs.captured(QStr(::status)));
                                auto cpuOverheadData = ::format(matchs.captured(QStr(::cpuOverhead)));
                                auto memoryOverheadData = ::format(matchs.captured(QStr(::memoryOverhead)));
                                auto timeCountData = ::format(matchs.captured(QStr(::timeCount)));
                                auto commandData = ::format(matchs.captured(QStr(::command)));
                                if (!processIdData.isEmpty() && !userData.isEmpty()
                                        && !priorityData.isEmpty() && !niceData.isEmpty()
                                        && !virtualSizeData.isEmpty() && !residentSizeData.isEmpty()
                                        && !sharableData.isEmpty() && !statusData.isEmpty()
                                        && !cpuOverheadData.isEmpty() && !memoryOverheadData.isEmpty()
                                        && !timeCountData.isEmpty() && !commandData.isEmpty()) {

                                    Json::Value globalLine(Json::arrayValue);
                                    globalLine.resize(feildArray.size());
                                    for (unsigned int idx = 0; idx < feildArray.size(); idx ++) {
                                        auto feild = feildArray[idx];
                                        if (feild == ::processId) {
                                            globalLine[idx] = processIdData.toStdString();
                                        } else if (feild == ::user) {
                                            globalLine[idx] = userData.toStdString();
                                        } else if (feild == ::priority) {
                                            globalLine[idx] = priorityData.toStdString();
                                        } else if (feild == ::nice) {
                                            globalLine[idx] = niceData.toStdString();
                                        } else if (feild == ::virtualSize) {
                                            globalLine[idx] = virtualSizeData.toStdString();
                                        } else if (feild == ::residentSize) {
                                            globalLine[idx] = residentSizeData.toStdString();
                                        } else if (feild == ::sharable) {
                                            globalLine[idx] = sharableData.toStdString();
                                        } else if (feild == ::status) {
                                            globalLine[idx] = statusData.toStdString();
                                        } else if (feild == ::cpuOverhead) {
                                            globalLine[idx] = cpuOverheadData.toStdString() + "%";
                                        } else if (feild == ::memoryOverhead) {
                                            globalLine[idx] = memoryOverheadData.toStdString() + "%";
                                        } else if (feild == ::timeCount) {
                                            globalLine[idx] = timeCountData.toStdString();
                                        } else if (feild == ::command) {
                                            globalLine[idx] = commandData.toStdString();
                                        } else {
                                            qCritical() << "unknown feild from performance type : global";
                                        }
                                    }

                                    Json::Value linesArray(Json::arrayValue);
                                    linesArray.append(globalLine);
                                    globalObj[::lines] = linesArray;
                                }
                            }
                        }

                        if (vfs == type) {
                            Json::Value feildArray = vfsObj[::feild];
                            if (!feildArray.empty() && feildArray.isArray()) {
                                auto commandData = ::format(matchs.captured(QStr(::command)));
                                auto processIdData = ::format(matchs.captured(QStr(::processId)));
                                auto userData = ::format(matchs.captured(QStr(::user)));
                                auto fdData = ::format(matchs.captured(QStr(::fd)));
                                auto typeData = ::format(matchs.captured(QStr(::type)));
                                auto deviceData = ::format(matchs.captured(QStr(::device)));
                                auto sizeOffData = ::format(matchs.captured(QStr(::sizeOff)));
                                auto nodeData = ::format(matchs.captured(QStr(::node)));
                                auto nameData = ::format(matchs.captured(QStr(::name)));
                                if (!commandData.isEmpty() && !processIdData.isEmpty()
                                        && !userData.isEmpty() && !fdData.isEmpty()
                                        && !typeData.isEmpty() && !deviceData.isEmpty()
                                        && !sizeOffData.isEmpty() && !nodeData.isEmpty()
                                        && !nameData.isEmpty()) {

                                    Json::Value linesArray = vfsObj[::lines];
                                    Json::Value vfsLine(Json::arrayValue);
                                    vfsLine.resize(feildArray.size());
                                    for (unsigned int idx = 0; idx < feildArray.size(); idx ++) {
                                        auto feild = feildArray[idx];
                                        if (feild == ::command) {
                                            vfsLine[idx] = commandData.toStdString();
                                        } else if (feild == ::processId) {
                                            vfsLine[idx] = processIdData.toStdString();
                                        } else if (feild == ::user) {
                                            vfsLine[idx] = userData.toStdString();
                                        } else if (feild == ::fd) {
                                            vfsLine[idx] = fdData.toStdString();
                                        } else if (feild == ::type) {
                                            vfsLine[idx] = typeData.toStdString();
                                        } else if (feild == ::device) {
                                            vfsLine[idx] = deviceData.toStdString();
                                        } else if (feild == ::sizeOff) {
                                            vfsLine[idx] = sizeOffData.toStdString();
                                        } else if (feild == ::node) {
                                            vfsLine[idx] = nodeData.toStdString();
                                        } else if (feild == ::name) {
                                            vfsLine[idx] = nameData.toStdString();
                                        } else {
                                            qCritical() << "unknown feild from performance type : global";
                                        }
                                    }
                                    linesArray.append(vfsLine);
                                    vfsObj[::lines] = linesArray;
                                }
                            }
                        }

                        if (socket == type) {
                            Json::Value feildArray = socketObj[::feild];
                            if (!feildArray.empty() && feildArray.isArray()) {
                                auto netIdData = ::format(matchs.captured(QStr(::netId)));
                                auto stateData = ::format(matchs.captured(QStr(::state)));
                                auto recvQueueData = ::format(matchs.captured(QStr(::recvQueue)));
                                auto sendQueueData = ::format(matchs.captured(QStr(::sendQueue)));
                                auto localAddrPortData = ::format(matchs.captured(QStr(::localAddrPort)));
                                auto peerAddrPortData = ::format(matchs.captured(QStr(::peerAddrPort)));
                                auto usingProcessesData = ::format(matchs.captured(QStr(::usingProcesses)));
                                // maybe usingProcesses is empty, also to do.
                                if (!netIdData.isEmpty() && !stateData.isEmpty()
                                        && !recvQueueData.isEmpty() && !sendQueueData.isEmpty()
                                        && !localAddrPortData.isEmpty() && !peerAddrPortData.isEmpty())
                                {
                                    Json::Value socketOne;
                                    socketOne[::netId] = netIdData.toStdString();
                                    socketOne[::state] = stateData.toStdString();
                                    socketOne[::recvQueue] = recvQueueData.toStdString();
                                    socketOne[::sendQueue] = sendQueueData.toStdString();
                                    socketOne[::localAddrPort] = localAddrPortData.toStdString();
                                    socketOne[::peerAddrPort] = peerAddrPortData.toStdString();
                                    socketOne[::usingProcesses] = usingProcessesData.toStdString();

                                    Json::Value linesArray = socketObj[::lines];
                                    Json::Value socketLine(Json::arrayValue);
                                    socketLine.resize(feildArray.size());
                                    for (unsigned int idx = 0; idx < feildArray.size(); idx ++) {
                                        auto feild = feildArray[idx];
                                        if (feild == ::netId) {
                                            socketLine[idx] = netIdData.toStdString();
                                        } else if (feild == ::state) {
                                            socketLine[idx] = stateData.toStdString();
                                        } else if (feild == ::recvQueue) {
                                            socketLine[idx] = recvQueueData.toStdString();
                                        } else if (feild == ::sendQueue) {
                                            socketLine[idx] = sendQueueData.toStdString();
                                        } else if (feild == ::localAddrPort) {
                                            socketLine[idx] = localAddrPortData.toStdString();
                                        } else if (feild == ::peerAddrPort) {
                                            socketLine[idx] = peerAddrPortData.toStdString();
                                        } else if (feild == ::usingProcesses) {
                                            socketLine[idx] = usingProcessesData.toStdString();
                                        } else {
                                            qCritical() << "unknown feild from performance type : global";
                                        }
                                    }
                                    linesArray.append(socketLine);
                                    socketObj[::lines] = linesArray;
                                }
                            }
                        }

                        if (deviceIO == type) {
                            Json::Value feildArray = deviceIOObj[::feild];
                            if (!feildArray.empty() && feildArray.isArray()) {
                                auto userIdData = ::format(matchs.captured(QStr(::userId)));
                                auto processIdData = ::format(matchs.captured(QStr(::processId)));
                                auto kB_rdData = ::format(matchs.captured(QStr(::kB_rd)));
                                auto kB_wdData = ::format(matchs.captured(QStr(::kB_wd)));
                                auto kB_ccwrData = ::format(matchs.captured(QStr(::kB_ccwr)));
                                auto iodelayData = ::format(matchs.captured(QStr(::iodelay)));
                                auto commandData = ::format(matchs.captured(QStr(::command)));
                                if (!userIdData.isEmpty() && !processIdData.isEmpty()
                                        && !kB_rdData.isEmpty() && !kB_wdData.isEmpty()
                                        && !kB_ccwrData.isEmpty() && !iodelayData.isEmpty()
                                        && !commandData.isEmpty())
                                {
                                    Json::Value linesArray;
                                    Json::Value deviceIOLine(Json::arrayValue);
                                    deviceIOLine.resize(feildArray.size());
                                    for (unsigned int idx = 0; idx < feildArray.size(); idx ++) {
                                        auto feild = feildArray[idx];
                                        if (feild == ::userId) {
                                            deviceIOLine[idx] = userIdData.toStdString();
                                        } else if (feild == ::processId) {
                                            deviceIOLine[idx] = processIdData.toStdString();
                                        } else if (feild == ::kB_rd + "/s") {
                                            deviceIOLine[idx] = kB_rdData.toStdString();
                                        } else if (feild == ::kB_wd + "/s") {
                                            deviceIOLine[idx] = kB_wdData.toStdString();
                                        } else if (feild == ::kB_ccwr + "/s") {
                                            deviceIOLine[idx] = kB_ccwrData.toStdString();
                                        } else if (feild == ::iodelay) {
                                            deviceIOLine[idx] = iodelayData.toStdString();
                                        } else if (feild == ::command) {
                                            deviceIOLine[idx] = commandData.toStdString();
                                        } else {
                                            qCritical() << "unknown feild from performance type : global";
                                        }
                                    }
                                    linesArray.append(deviceIOLine);
                                    deviceIOObj[::lines] = linesArray;
                                }
                            }
                        }
                    }
                }
            }

            if (!functionObj[::lines].empty()) {
                d->countFunction(functionObj); // count function information
            }

            if (!vfsObj[::lines].empty()) {
                d->toolsCountCache[PerformanceType::vfs] = vfsObj;
            }

            if (!socketObj[::lines].empty()) {
                d->toolsCountCache[PerformanceType::socket] = socketObj;
            }

            if (!deviceIOObj[::lines].empty()) {
                d->toolsCountCache[PerformanceType::deviceIO] = deviceIOObj;
            }

            if (!globalObj[::lines].empty()) { // with global timer get count function information
                Json::Value result;
                d->toolsCountCache[PerformanceType::global] = globalObj;
                Json::Value countfunctionCache = d->toolsCountCache[PerformanceType::function];
                if (!countfunctionCache[::lines].empty()) {
                    d->toolsCountCache[PerformanceType::function] = countfunctionCache;
                    result[::enumElemName(PerformanceType::function)] = countfunctionCache;
                }
                Json::Value vfsCache = d->toolsCountCache[PerformanceType::vfs];
                if (!vfsCache[::lines].empty()) {
                    result[::enumElemName(PerformanceType::vfs)] = vfsCache;
                }
                Json::Value socketCache = d->toolsCountCache[PerformanceType::socket];
                if (!socketCache[::lines].empty()) {
                    result[::enumElemName(PerformanceType::socket)] = socketCache;
                }
                Json::Value deviceIOCache = d->toolsCountCache[PerformanceType::deviceIO];
                if (!deviceIOCache[::lines].empty()) {
                    result[::enumElemName(PerformanceType::deviceIO)] = deviceIOCache;
                }

                result[::enumElemName(PerformanceType::global)] = globalObj;
                qInfo() << qPrintable(QStr(result.toStyledString()));
                emit attachData(result);
            }
        }
    }
}

void Tools::readAllStandardError()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (process) {
        QString errorOut = process->readAllStandardError().replace("\n", " ");
        //        if (process->error() == QProcess::UnknownError) {
        //            PerformanceType pt = qvariant_cast<PerformanceType>(process->property(::enumName<PerformanceType>()));
        //            QString program = process->program();
        //            QString package_name = process->property(::package_name.c_str()).toString();

        //            d->displayErrorWidget()->append(QStr(::enumName<PerformanceType>()) + ": " + ::enumElemName<PerformanceType>(pt));
        //            d->displayErrorWidget()->append(QStr("program: ") + process->program() + process->arguments().join(" "));
        //            d->displayErrorWidget()->append(QStr("errorString: ") + errorOut);
        //            if (package_name.toStdString() != ::system_default) {
        //                d->displayErrorWidget()->append(Tools::tr("suggestion: "
        //                                                          "please to install tool, the package-name \"%0\". "
        //                                                          "If you confirm that the current tool exists and still cannot be used normally, "
        //                                                          "the current operation does not support").arg(package_name));
        //            } else {
        //                d->displayErrorWidget()->append(QStr("suggestion: ")
        //                                                + "Because the current program is a system application, "
        //                                                  "if it is not found, "
        //                                                  "it means that the item is unavailable");
        //            }
        //            d->displayErrorWidget()->append("\n");
        //            d->displayErrorWidget()->show();
        //        }
        qCritical() << qvariant_cast<PerformanceType>(process->property(::enumName<PerformanceType>()))
                    << errorOut
                    << process->error()
                    << process->errorString();
    }
}

void Tools::errorOccurred(QProcess::ProcessError error)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (process) {

        PerformanceType pt = qvariant_cast<PerformanceType>(process->property(::enumName<PerformanceType>()));
        QString program = process->program();
        QString package_name = process->property(::package_name.c_str()).toString();

        d->displayErrorWidget()->append(QStr(::enumName<PerformanceType>()) + ": " + ::enumElemName<PerformanceType>(pt));
        d->displayErrorWidget()->append(QStr("program: ") + process->program() + process->arguments().join(" "));
        d->displayErrorWidget()->append(QStr("errorString: ") + process->errorString());
        if (package_name.toStdString() != ::system_default) {
            d->displayErrorWidget()->append(Tools::tr("suggestion: "
                                                      "please to install tool, the package-name \"%0\". "
                                                      "If you confirm that the current tool exists and still cannot be used normally, "
                                                      "the current operation does not support").arg(package_name));
        } else {
            d->displayErrorWidget()->append(QStr("suggestion: ")
                                            + "Because the current program is a system application, "
                                              "if it is not found, "
                                              "it means that the item is unavailable");
        }
        d->displayErrorWidget()->append("\n");
        d->displayErrorWidget()->show();

        qCritical() << pt << error << process->errorString();
    }
}
