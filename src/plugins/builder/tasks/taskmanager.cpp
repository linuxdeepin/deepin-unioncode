// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskmanager.h"
#include "mainframe/settingdialog.h"
#include "transceiver/buildersender.h"
#include "common/common.h"
#include "services/option/optionmanager.h"
#include "services/ai/aiservice.h"

#include <QMenu>

using namespace dpfservice;

TaskManager *TaskManager::instance()
{
    static TaskManager ins;
    return &ins;
}

DListView *TaskManager::getView() const
{
    return view;
}

void TaskManager::clearTasks()
{
    model->clearTasks();
}

TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
{
    view = new TaskView();
    model.reset(new TaskModel());
    filterModel.reset(new TaskFilterProxyModel());
    filterModel->setSourceModel(model.get());
    view->setModel(filterModel.get());
    auto tld = new TaskDelegate(view);
    view->setItemDelegate(tld);

    view->setFrameStyle(QFrame::NoFrame);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(view->selectionModel(), &QItemSelectionModel::currentChanged,
            tld, &TaskDelegate::currentChanged);

    connect(view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &TaskManager::currentChanged);
    connect(view, &QAbstractItemView::activated,
            this, &TaskManager::triggerDefaultHandler);
    connect(view, &TaskView::customContextMenuRequested,
            this, &TaskManager::showContextMenu);
}

QString TaskManager::readContext(const QString &path, int codeLine)
{
    QStringList context;
    int startLine = qMax(0, codeLine - 3);
    int endLine = codeLine + 3;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        int line = 0;
        while (!in.atEnd()) {
            const QString lineContent = in.readLine();
            if (line >= startLine && line <= endLine)
                context.append(lineContent);
            ++line;
        }
        endLine = line - 1;
        file.close();
    }

    if (!context.isEmpty()) {
        QString prefix("```%1 (%2-%3)");
        context.prepend(prefix.arg(path, QString::number(startLine + 1), QString::number(endLine + 1)));
        context.append("```");
    }

    return context.join('\n');
}

void TaskManager::slotAddTask(const Task &task, int linkedOutputLines, int skipLines)
{
    Q_UNUSED(linkedOutputLines)
    Q_UNUSED(skipLines)

    model->addTask(task);
}

void TaskManager::showSpecificTasks(ShowType type)
{
    filterModel->setFilterType(type);
}

void TaskManager::showContextMenu()
{
    QMenu menu;
    menu.addAction(tr("Clear"), this, &TaskManager::clearTasks);
    auto act = menu.addAction(tr("Fix Issue"), this, &TaskManager::fixIssueWithAi);
    
    auto pos = QCursor::pos();
    if (!view->indexAt(view->mapFromGlobal(pos)).isValid())
        act->setEnabled(false);

    menu.exec(pos);
}

void TaskManager::fixIssueWithAi()
{
    auto realIndex = filterModel->mapToSource(view->currentIndex());
    const auto &task = model->task(realIndex);
    if (task.isNull())
        return;

    QString context;
    if (!task.file.toString().isEmpty() && task.line > 0)
        context = readContext(task.file.toString(), task.line - 1);

    QString prompt = context + "\n\n";
    const auto title = OptionManager::getInstance()->getValue("Builder", "CurrentPrompt").toString();
    if (title.isEmpty()) {
        prompt += SettingDialog::defaultIssueFixPrompt() + '\n';
    } else {
        const auto prompts = OptionManager::getInstance()->getValue("Builder", "Prompts").toMap();
        prompt += prompts.value(title).toString() + '\n';
    }

    prompt += task.description;
    if (!aiSrv)
        aiSrv = dpfGetService(AiService);
    aiSrv->chatWithAi(prompt);
}

void TaskManager::currentChanged(const QModelIndex &index)
{
    Q_UNUSED(index)
}

void TaskManager::triggerDefaultHandler(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    auto realIndex = filterModel->mapToSource(index);
    Task task(model->task(realIndex));
    if (task.isNull())
        return;

    if (task.file.exists()) {
        editor.gotoLine(task.file.toString(), task.movedLine - 1);
    }
}
