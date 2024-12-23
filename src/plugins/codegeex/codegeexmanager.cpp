// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexmanager.h"
#include "copilot.h"
#include "common/util/custompaths.h"
#include "services/window/windowservice.h"
#include "services/window/windowelement.h"
#include "services/terminal/terminalservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionmanager.h"
#include "services/editor/editorservice.h"

#include <DSpinner>

#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QJsonDocument>
#include <QMultiMap>
#include <QUuid>
#include <QTimer>
#include <QDateTime>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>

// might useful later.
//static const char *kUrlQueryMessage = "https://codegeex.cn/prod/code/chatGmlMsg/selectList";

static const QString PrePrompt = "Use the above <context></context> code to answer the following question. You should not reference any files outside of what is shown, unless they are commonly known files, like a .gitignore or package.json. Reference the filenames whenever possible. If there isn't enough information to answer the question, suggest where the user might look to learn more";

using namespace CodeGeeX;
using namespace dpfservice;

QJsonArray parseFile(QStringList files)
{
    QJsonArray result;
    auto editorSrv = dpfGetService(EditorService);

    for (auto file : files) {
        QJsonObject obj;
        obj["name"] = QFileInfo(file).fileName();
        obj["language"] = support_file::Language::id(file);

        QString fileContent = editorSrv->fileText(file);

        if (fileContent.isEmpty()) {
            QFile content(file);
            if (content.open(QIODevice::ReadOnly)) {
                obj["content"] = QString(content.read(20000));
            }
        } else {
            obj["content"] = QString(fileContent.mid(0, 20000));
        }
        result.append(obj);
    }

    return result;
}

CodeGeeXManager *CodeGeeXManager::instance()
{
    static CodeGeeXManager ins;
    return &ins;
}

void CodeGeeXManager::checkCondaInstalled()
{
    if (condaInstalled)
        return;
    QProcess process;
    QStringList arguments;
    arguments << "env"
              << "list";

    process.start(condaRootPath() + "/miniforge/condabin/conda", arguments);
    process.waitForFinished();

    QString output = process.readAll();
    condaInstalled = output.contains("deepin_unioncode_env");
}

bool CodeGeeXManager::condaHasInstalled()
{
    return condaInstalled;
}

AbstractLLM *CodeGeeXManager::getCurrentLLM() const
{
    return chatLLM;
}

void CodeGeeXManager::setLocale(CodeGeeX::Locale locale)
{
    this->locale = locale;
    chatLLM->setLocale(locale == CodeGeeX::Zh ? AbstractLLM::Zh : AbstractLLM::En);
    Copilot::instance()->setLocale(locale);
}

void CodeGeeXManager::connectToNetWork(bool connecting)
{
    networkEnabled = connecting;
}

bool CodeGeeXManager::isConnectToNetWork() const
{
    return networkEnabled;
}

void CodeGeeXManager::setReferenceCodebase(bool on)
{
    codebaseEnabled = on;
}

bool CodeGeeXManager::isReferenceCodebase() const
{
    return codebaseEnabled;
}

void CodeGeeXManager::setReferenceFiles(const QStringList &files)
{
    referenceFiles = files;
}

QStringList CodeGeeXManager::getReferenceFiles()
{
    return referenceFiles;
}

void CodeGeeXManager::deleteCurrentSession()
{
    auto c = chatLLM->getCurrentConversation();
    c->clear();
}

void CodeGeeXManager::deleteSession(const QString &talkId)
{
}

void CodeGeeXManager::setMessage(const QString &prompt)
{
    Q_EMIT setTextToSend(prompt);
}

QString CodeGeeXManager::getChunks(const QString &queryText)
{
    using dpfservice::ProjectService;
    ProjectService *prjService = dpfGetService(ProjectService);
    auto currentProjectPath = prjService->getActiveProjectInfo().workspaceFolder();

    if (currentProjectPath != "") {
        QJsonObject result = CodeGeeXManager::instance()->query(currentProjectPath, queryText, 20);
        QJsonArray chunks = result["Chunks"].toArray();
        if (!chunks.isEmpty()) {
            if (result["Completed"].toBool() == false)
                emit notify(0, tr("The indexing of project %1 has not been completed, which may cause the results to be inaccurate.").arg(currentProjectPath));
            QString context;
            context += "\n<context>\n";
            for (auto chunk : chunks) {
                context += chunk.toObject()["fileName"].toString();
                context += '\n```';
                context += chunk.toObject()["content"].toString();
                context += "```\n\n";
            }
            context += "\n</context>";
            return context;
        } else if (CodeGeeXManager::instance()->condaHasInstalled()) {
            emit noChunksFounded();
            return "";
        }
    }

    return "";
}

QString CodeGeeXManager::promptPreProcessing(const QString &originText)
{
    QString processedText = PrePrompt;

    QString message = originText;
#ifdef SUPPORTMINIFORGE
    if (isReferenceCodebase()) {
        if (!condaHasInstalled()) {   // if not x86 or arm. @codebase can not be use
            QStringList actions { "ai_rag_install", tr("Install") };
            emit notify(0, CodeGeeXManager::tr("The file indexing feature is not available, which may cause functions such as @codebase to not work properly."
                                                                       "Please install the required environment.\n the installation process may take several minutes."),
                                          actions);
        } else {
            QString prompt = QString("Translate this passage into English :\"%1\", with the requirements: Do not provide responses other than translation.").arg(message.remove("@CodeBase"));
            auto englishPrompt = requestSync(prompt);
            QString chunksContext = getChunks(englishPrompt);
            if (chunksContext.isEmpty())
                return "";
            if (message.contains("@CodeBase"))
                message.remove("@CodeBase");
            processedText.append("\n<context>\n" + chunksContext + "\n</context>");
        }
    }
#endif
    if (!getReferenceFiles().isEmpty()) {
        QJsonArray files = parseFile(getReferenceFiles());
        for (auto file : files) {
            auto fileName = file.toObject()["name"].toString();
            auto language = file.toObject()["language"].toString();
            auto content = file.toObject()["content"].toString();

            processedText.append("\n" + fileName + "\n```" + language + "\n" + content + "```\n");
        }
    }

    if (processedText != PrePrompt) {
        processedText.append("\n\n" + message);
        if (locale == CodeGeeX::Zh)
            processedText.append("\nPlease answer me by Chinese");
        return processedText;
    } else {
        return originText;
    }
}

void CodeGeeXManager::sendMessage(const QString &prompt)
{
    if (!chatLLM) {
        emit notify(2, tr("No selected LLM or current LLM is not avaliable"));
        return;
    }

    QString askId = "User" + QString::number(QDateTime::currentMSecsSinceEpoch());
    MessageData msgData(askId, MessageData::Ask);
    msgData.updateData(prompt);
    Q_EMIT requestMessageUpdate(msgData);

    requestAsync(prompt);
}

//For chatting: Using user-defined models
void CodeGeeXManager::requestAsync(const QString &prompt)
{
    if (!chatLLM || chatLLM->modelState() == AbstractLLM::Busy)
        return;

    answerFlag++;
    startReceiving();
    QtConcurrent::run([=](){
        auto processedText = promptPreProcessing(prompt);
        if (processedText.isEmpty())
            return;
        auto c = chatLLM->getCurrentConversation();
        c->addUserData(processedText);
        QJsonObject obj = chatLLM->create(*c);
        if (isConnectToNetWork())
            obj.insert("command", "online_search_v1"); // only worked on CodeGeeX llm
        emit sendSyncRequest(obj);
    });
}

//For quick processing of special requests
QString CodeGeeXManager::requestSync(const QString &prompt)
{
    QEventLoop loop;
    QString response;
    connect(liteLLM, &AbstractLLM::dataReceived, &loop, [=, &loop, &response](const QString &data, AbstractLLM::ResponseState state) {
        response = data;
        loop.exit();
    });
    // use liteLLM to handle request. quickly get the response
    liteLLM->request(prompt);
    loop.exec();
    return response;
}

void CodeGeeXManager::slotSendSyncRequest(const QJsonObject &obj)
{
    if (chatLLM)
        chatLLM->request(obj);
    else
        emit notify(2, tr("No selected LLM or current LLM is not avaliable"));
}

void CodeGeeXManager::onResponse(const QString &msgID, const QString &data, AbstractLLM::ResponseState state)
{
    if (msgID.isEmpty())
        return;

    if (state == AbstractLLM::ResponseState::Canceled || state == AbstractLLM::Failed) {
        return;
    }

    auto msgData = modifiedData(data);
    if (state == AbstractLLM::ResponseState::Receiving) {
        responseData += msgData;
        if (!curSessionMsg.contains(msgID))
            curSessionMsg.insert(msgID, MessageData(msgID, MessageData::Anwser));

        if (!data.isEmpty()) {
            curSessionMsg[msgID].updateData(responseData);
            Q_EMIT requestMessageUpdate(curSessionMsg[msgID]);
        }
    } else {
        if (responseData.isEmpty() && !data.isEmpty()) {
            responseData = msgData;
            if (!curSessionMsg.contains(msgID))
                curSessionMsg.insert(msgID, MessageData(msgID, MessageData::Anwser));
            curSessionMsg[msgID].updateData(responseData);
            Q_EMIT requestMessageUpdate(curSessionMsg[msgID]);
        }

        responseData.clear();
        isRunning = false;
        emit chatFinished();
        return;
    }
}

void CodeGeeXManager::onLLMChanged(const LLMInfo &llmInfo)
{
    if (chatLLM) {
        if (chatLLM->modelState() == AbstractLLM::Busy) {
            emit terminated();
            stopReceiving();
            chatLLM->cancel();
        }
        disconnect(chatLLM, &AbstractLLM::dataReceived, this, nullptr);
        disconnect(chatLLM, &AbstractLLM::customDataReceived, this, nullptr);
        disconnect(this, &CodeGeeXManager::requestStop, chatLLM, &AbstractLLM::cancel);
    };

    auto aiSrv = dpfGetService(AiService);
    auto selectedLLM = aiSrv->getLLM(llmInfo);
    if (!selectedLLM) {
        QString error = tr("llm named: %1 is not avaliable.").arg(llmInfo.modelName);
        emit notify(1, error);
        return;
    }

    chatLLM = selectedLLM;
    initLLM(chatLLM);
    emit llmChanged(llmInfo);
}

void CodeGeeXManager::recevieDeleteResult(const QStringList &talkIds, bool success)
{
    if (success) {
        for (const QString &talkId : talkIds) {
            int i = 0;
            while (i < sessionRecordList.length()) {
                if (sessionRecordList[i].talkId == talkId)
                    sessionRecordList.removeAt(i);
                else
                    ++i;
            }
        }
        Q_EMIT sessionRecordsUpdated();
    } else {
        qWarning() << "Delete history session failed!";
    }
}

CodeGeeXManager::CodeGeeXManager(QObject *parent)
    : QObject(parent)
{
    auto aiSrv = dpfGetService(AiService);
    auto liteLLMInfo = aiSrv->getCodeGeeXLLMLite();
    liteLLM = aiSrv->getLLM(liteLLMInfo);
    liteLLM->setStream(false);

    initConnections();
}

void CodeGeeXManager::initConnections()
{
    connect(this, &CodeGeeXManager::noChunksFounded, this, &CodeGeeXManager::showIndexingWidget);

    connect(Copilot::instance(), &Copilot::messageSended, this, &CodeGeeXManager::startReceiving);

    connect(this, &CodeGeeXManager::requestStop, Copilot::instance(), &Copilot::requestStop);
    connect(this, &CodeGeeXManager::notify, this, [](int type, const QString &message, QStringList actions) {
        WindowService *windowService = dpfGetService(WindowService);
        windowService->notify(type, "Ai", message, actions);
    });
    connect(this, &CodeGeeXManager::sendSyncRequest, this, &CodeGeeXManager::slotSendSyncRequest);
    connect(this, &CodeGeeXManager::llmChanged, Copilot::instance(), [=](const LLMInfo &info){
        auto aiSrv = dpfGetService(AiService);
        auto copilotLLM = aiSrv->getLLM(info); // Use a new LLM to avoid affecting chatLLM
        if (copilotLLM)
            Copilot::instance()->setCopilotLLM(copilotLLM);
    });
}

void CodeGeeXManager::initLLM(AbstractLLM *llm)
{
    if (!llm)
        return;
    chatLLM->setLocale(locale == CodeGeeX::Zh ? AbstractLLM::Zh : AbstractLLM::En);
    connect(chatLLM, &AbstractLLM::dataReceived, this, [=](const QString &data, AbstractLLM::ResponseState state) {
        if (state == AbstractLLM::Canceled)
            return;
        if (state == AbstractLLM::Failed) {
            QString errStr;
            bool valid = chatLLM->checkValid(&errStr);
            if (!valid)
                emit notify(2, tr("LLM is not valid. %1").arg(errStr));
            else
                emit notify(2, tr("Error: %1, try again later").arg(data));
            return;
        }
        onResponse(QString::number(answerFlag), data, state);
    });
    connect(chatLLM, &AbstractLLM::customDataReceived, this, [=](const QString &key, const QJsonObject &obj) {
        QList<websiteReference> websites;
        if (key == "crawl") {
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                websiteReference website;
                QString citationKey = it.key();
                QJsonObject citationObj = it.value().toObject();

                website.citation = citationKey;
                website.status = citationObj["status"].toString();
                website.url = citationObj["url"].toString();
                website.title = citationObj["title"].toString();

                websites.append(website);
            }
            emit crawledWebsite(QString::number(answerFlag), websites);
        }
    });
    connect(this, &CodeGeeXManager::requestStop, chatLLM, &AbstractLLM::cancel);
}

// todo: storage of session records
void CodeGeeXManager::fetchSessionRecords()
{
}

void CodeGeeXManager::fetchMessageList(const QString &talkId)
{
}

void CodeGeeXManager::startReceiving()
{
    isRunning = true;
    emit chatStarted();
}

void CodeGeeXManager::stopReceiving()
{
    isRunning = false;
    responseData.clear();
    emit requestStop();
}

bool CodeGeeXManager::checkRunningState(bool state)
{
    return isRunning == state;
}

QList<RecordData> CodeGeeXManager::sessionRecords() const
{
    return sessionRecordList;
}

QString CodeGeeXManager::modifiedData(const QString &data)
{
    auto retData = data;
    retData.replace("\\n", "\n");
    retData.replace("\\\"", "\"");
    retData.replace("\\\\", "\\");

    return retData;
}

QString CodeGeeXManager::condaRootPath() const
{
    return QDir::homePath() + "/.unioncode";
}

void CodeGeeXManager::showIndexingWidget()
{
    emit chatFinished();

    auto widget = new QWidget;   // set parent in messageComponent
    auto layout = new QVBoxLayout(widget);

    const QString tip(tr("This project has not yet established a file index, @codebase wont`t work directly. Confirm whether to create one now."));
    auto label = new QLabel(tip, widget);
    label->setWordWrap(true);
    layout->addWidget(label);

    auto confirmBtn = new QPushButton(tr("Confirm"), widget);
    layout->addWidget(confirmBtn);

    using DTK_WIDGET_NAMESPACE::DSpinner;
    auto spinner = new DSpinner(widget);
    spinner->setFixedSize(16, 16);
    spinner->hide();

    using dpfservice::ProjectService;
    ProjectService *prjServ = dpfGetService(ProjectService);
    auto currentProject = prjServ->getActiveProjectInfo().workspaceFolder();
    connect(confirmBtn, &QPushButton::clicked, widget, [=]() {
        generateRag(currentProject);
        layout->addWidget(new QLabel(tr("It may take servel minutes"), widget));
        layout->addWidget(spinner);
        spinner->show();
        spinner->start();
        confirmBtn->setEnabled(false);
    });

    connect(this, &CodeGeeXManager::generateDone, spinner, [=](const QString &path, bool failed) {
        if (path == currentProject)
            spinner->hide();
        QString text = failed ? tr("Indexing Failed") : tr("Indexing Done");
        layout->addWidget(new QLabel(text, widget));
    });

    emit showCustomWidget(widget);
}

void CodeGeeXManager::installConda()
{
    if (installCondaTimer.isActive())
        return;

    QString scriptPath = CustomPaths::CustomPaths::global(CustomPaths::Scripts) + "/rag/install.sh";
    QProcess process;
    process.setProgram("ps");
    process.setArguments({ "aux" });
    process.start();
    process.waitForFinished();
    QString output = process.readAll();

    // check install script is running
    bool exists = output.contains(scriptPath);
    if (exists)
        return;
    auto terminalServ = dpfGetService(dpfservice::TerminalService);
    WindowService *windowService = dpfGetService(WindowService);
    windowService->switchContextWidget(dpfservice::TERMINAL_TAB_TEXT);
    terminalServ->executeCommand("install", "bash", QStringList() << scriptPath << condaRootPath(), condaRootPath(), QStringList());

    installCondaTimer.setSingleShot(true);   // terminal may not execute it immediately. add timer to Prevent multiple triggers within a short period of time.
    installCondaTimer.start(2000);
}

void CodeGeeXManager::generateRag(const QString &projectPath)
{
    mutex.lock();
    if (indexingProject.contains(projectPath)) {
        mutex.unlock();
        return;
    }
    indexingProject.append(projectPath);
    mutex.unlock();
    QProcess *process = new QProcess;
    QObject::connect(QApplication::instance(), &QApplication::aboutToQuit, process, [process]() {
        process->kill();
    },
                     Qt::DirectConnection);

    QObject::connect(process, &QProcess::readyReadStandardError, process, [process]() {
        qInfo() << "Error:" << process->readAllStandardError() << "\n";
    });

    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     process, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                         qInfo() << "Python script finished with exit code" << exitCode << "Exit!!!";
                         mutex.lock();
                         indexingProject.removeOne(projectPath);
                         mutex.unlock();
                         auto success = process->readAllStandardError().isEmpty();
                         emit generateDone(projectPath, !success);
                         if (!success)
                             emit notify(2, tr("The error occurred when performing rag on project %1.").arg(projectPath));
                         process->deleteLater();
                     });

    qInfo() << "start rag project:" << projectPath;

    QString ragPath = CustomPaths::CustomPaths::global(CustomPaths::Scripts) + "/rag";
    process->setWorkingDirectory(ragPath);
    auto generatePyPath = ragPath + "/generate.py";
    auto pythonPath = condaRootPath() + "/miniforge/envs/deepin_unioncode_env/bin/python";
    auto modelPath = CustomPaths::CustomPaths::global(CustomPaths::Models);
    if (!QFileInfo(pythonPath).exists())
        return;
    process->start(pythonPath, QStringList() << generatePyPath << modelPath << projectPath);
    if (QThread::currentThread() != qApp->thread())   // incase thread exit before process done. cause slot function can`t work
        process->waitForFinished();
}

/*
 JsonObject:
    Query: str
    Chunks: Arr[fileName:str, content:str]
    Instructions: obj{name:str, description:str, content:str}
*/
QJsonObject CodeGeeXManager::query(const QString &projectPath, const QString &query, int topItems)
{
    QProcess process;

    QObject::connect(&process, &QProcess::readyReadStandardError, &process, [&]() {
        qInfo() << "Error:" << process.readAllStandardError() << "\n";
    });

    // If modified, update the Python file accordingly.
    auto pythonPath = condaRootPath() + "/miniforge/envs/deepin_unioncode_env/bin/python";
    if (!QFileInfo(pythonPath).exists() || !QFileInfo(condaRootPath() + "/index.sqlite").exists())
        return {};

    auto modelPath = CustomPaths::CustomPaths::global(CustomPaths::Models);
    auto ragPath = CustomPaths::CustomPaths::global(CustomPaths::Scripts) + "/rag";
    process.setWorkingDirectory(ragPath);
    auto queryPyPath = ragPath + "/query.py";
    process.start(pythonPath, QStringList() << queryPyPath << modelPath << projectPath << query << QString::number(topItems));
    process.waitForFinished();
    auto result = process.readAll();
    QJsonDocument document = QJsonDocument::fromJson(result);

    QJsonObject obj = document.object();
    if (indexingProject.contains(projectPath))
        obj["Completed"] = false;
    else
        obj["Completed"] = true;

    if (isReferenceCodebase())
        currentChunks = obj;
    else
        currentChunks = QJsonObject();

    return obj;
}
