// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lexermanager.h"
#include "scilexerhandler.h"

#include "services/editor/editorservice.h"

#include <QDebug>

using namespace dpfservice;

LexerManager::LexerManager(QObject *parent)
    : QObject(parent)
{
}

LexerManager *LexerManager::instance()
{
    static LexerManager ins;
    return &ins;
}

void LexerManager::init(EditorService *editorSvc)
{
    using namespace std::placeholders;
    if (editorSvc)
        editorSvc->registerSciLexerProxy = std::bind(&LexerManager::registerSciLexerProxy, this, _1, _2);
}

void LexerManager::registerSciLexerProxy(const QString &language, AbstractLexerProxy *proxy)
{
    if (sciLexerProxyMng.contains(language)) {
        qWarning() << "The lexer proxy of " << language << " has been registed!";
        return;
    }

    if (!proxy) {
        qWarning() << "The proxy is null";
        return;
    }

    // The `proxy` is structured by `LexerManager`
    if (!proxy->parent())
        proxy->setParent(this);

    sciLexerProxyMng.insert(language, proxy);
}

QsciLexer *LexerManager::createSciLexer(const QString &language, const QString &fileName)
{
    auto proxy = sciLexerProxyMng.value(language, nullptr);
    if (proxy) {
        // The `SciLexerHandler` is structured by `TextEditor`
        SciLexerHandler *handler = new SciLexerHandler();
        handler->setProxy(proxy);
        return handler;
    }

    for (auto proxy : sciLexerProxyMng) {
        if (proxy->isSupport(fileName)) {
            SciLexerHandler *handler = new SciLexerHandler();
            handler->setProxy(proxy);
            return handler;
        }
    }

    return defaultSciLexer(language);
}

QsciLexer *LexerManager::defaultSciLexer(const QString &language)
{
    QsciLexer *lexer { nullptr };
    if (language.compare("cpp", Qt::CaseInsensitive) == 0) {
        lexer = new QsciLexerCPP();
    } else if (language.compare("java", Qt::CaseInsensitive) == 0) {
        lexer = new QsciLexerJava();
    } else if (language.compare("cmake", Qt::CaseInsensitive) == 0) {
        lexer = new QsciLexerCMake();
    } else if (language.compare("json", Qt::CaseInsensitive) == 0) {
        lexer = new QsciLexerJSON();
    } else if (language.compare("xml", Qt::CaseInsensitive) == 0) {
        lexer = new QsciLexerXML();
    } else if (language.compare("python", Qt::CaseInsensitive) == 0) {
        lexer = new QsciLexerPython();
    } else if (language.compare("js", Qt::CaseInsensitive) == 0) {
        lexer = new QsciLexerJavaScript();
    } else {
        lexer = new QsciLexerCPP();
    }

    // The `lexer` is structured by `TextEditor`
    return lexer;
}
