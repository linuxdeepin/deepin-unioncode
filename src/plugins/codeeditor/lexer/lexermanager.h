// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEXERMANAGER_H
#define LEXERMANAGER_H

#include "defaultlexer.h"
#include "base/abstractlexerproxy.h"

namespace dpfservice {
class EditorService;
}

class LexerManager : public QObject
{
    Q_OBJECT
public:
    static LexerManager *instance();

    void init(dpfservice::EditorService *editorSvc);
    void registerSciLexerProxy(const QString &language, AbstractLexerProxy *proxy);
    QsciLexer *createSciLexer(const QString &language, const QString &fileName);
    QsciLexer *defaultSciLexer(const QString &language);

private:
    explicit LexerManager(QObject *parent = nullptr);

    QHash<QString, AbstractLexerProxy *> sciLexerProxyMng;
};

#endif   // LEXERMANAGER_H
