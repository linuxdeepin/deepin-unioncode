// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORSERVICE_H
#define EDITORSERVICE_H

#include <framework/framework.h>
#include "base/abstractdebugger.h"
#include "base/abstractlexerproxy.h"

namespace dpfservice {

class EditorService final : public dpf::PluginService,
        dpf::AutoServiceRegister<EditorService>
{
    Q_OBJECT
    Q_DISABLE_COPY(EditorService)

public:
    explicit EditorService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    static QString name()
    {
        return "org.deepin.service.EditorService";
    }

    DPF_INTERFACE(QString, getSelectedText);
    DPF_INTERFACE(QString, getCursorBeforeText);
    DPF_INTERFACE(QString, getCursorBehindText);
    DPF_INTERFACE(void, replaceSelectedText, const QString &);
    DPF_INTERFACE(void, insertText, const QString &);
    DPF_INTERFACE(void, showTips, const QString &tips);
    DPF_INTERFACE(void, undo);

    DPF_INTERFACE(void, registSciLexerProxy, const QString &language, AbstractLexerProxy *proxy);
};

} // namespace dpfservice
#endif // EDITORSERVICE_H
