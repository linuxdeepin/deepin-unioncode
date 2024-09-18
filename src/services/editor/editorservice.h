// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORSERVICE_H
#define EDITORSERVICE_H

#include <framework/framework.h>
#include "base/abstractdebugger.h"
#include "base/abstractlexerproxy.h"
#include "base/abstracteditwidget.h"

namespace dpfservice {

class EditorService final : public dpf::PluginService,
                            dpf::AutoServiceRegister<EditorService>
{
    Q_OBJECT
    Q_DISABLE_COPY(EditorService)

public:
    explicit EditorService(QObject *parent = nullptr)
        : dpf::PluginService(parent)
    {
    }

    static QString name()
    {
        return "org.deepin.service.EditorService";
    }

    DPF_INTERFACE(QString, getSelectedText);
    DPF_INTERFACE(QString, getCursorBeforeText);
    DPF_INTERFACE(QString, getCursorBehindText);
    DPF_INTERFACE(QStringList, modifiedFiles);
    DPF_INTERFACE(void, saveAll);
    DPF_INTERFACE(void, replaceSelectedText, const QString &);
    DPF_INTERFACE(void, insertText, const QString &);
    DPF_INTERFACE(void, showTips, const QString &tips);
    DPF_INTERFACE(void, undo);
    DPF_INTERFACE(void, setText, const QString &text);
    DPF_INTERFACE(void, setCompletion, const QString &info, const QIcon &icon, const QKeySequence &key);
    DPF_INTERFACE(QString, currentFile);
    DPF_INTERFACE(QStringList, openedFiles);
    DPF_INTERFACE(QString, fileText, const QString &file);
    DPF_INTERFACE(void, cursorPosition, int *line, int *index);
    DPF_INTERFACE(void, replaceAll, const QString &file, const QString &oldText,
                  const QString &newText, bool caseSensitive, bool wholeWords);
    DPF_INTERFACE(void, replaceRange, const QString &file, int line, int index, int length, const QString &after);

    DPF_INTERFACE(void, registerSciLexerProxy, const QString &language, AbstractLexerProxy *proxy);
    DPF_INTERFACE(void, registerWidget, const QString &id, AbstractEditWidget *widget);
    DPF_INTERFACE(void, switchWidget, const QString &id);
    DPF_INTERFACE(void, switchDefaultWidget);

    // NOTE: Return the `marker` value,
    //       if the return value is -1, it indicates that the setting failed.
    DPF_INTERFACE(int, setRangeBackgroundColor, const QString &file, int startLine, int endLine, const QColor &color);
    DPF_INTERFACE(void, clearRangeBackgroundColor, const QString &file, int startLine, int endLine, int marker);
    DPF_INTERFACE(void, clearAllBackgroundColor, const QString &file, int marker);
    DPF_INTERFACE(void, showLineWidget, int line, QWidget *widget);
    DPF_INTERFACE(void, closeLineWidget);

    using RepairCallback = std::function<void(const QString &info)>;
    using RepairToolInfo = QMap<QString, RepairCallback>;
    DPF_INTERFACE(void, registerDiagnosticRepairTool, const QString &toolName, RepairCallback callback);
    DPF_INTERFACE(RepairToolInfo, getDiagnosticRepairTool);
};

}   // namespace dpfservice
#endif   // EDITORSERVICE_H
