/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "textedit.h"
#include "lspclientkeeper.h"
#include "Document.h"
#include "SciLexer.h"
#include "common/common.h"
#include "framework/framework.h"
#include "transceiver/codeeditorreceiver.h"
#include "services/find/findservice.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QDir>
#include <QDebug>
#include <QLibrary>
#include <QApplication>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QTimer>

#include <bitset>
#include <iostream>
using namespace dpfservice;
class TextEditPrivate
{
    friend class TextEdit;
    StyleLsp *styleLsp {nullptr};
    StyleSci *styleSci {nullptr};
};

TextEdit::TextEdit(QWidget *parent)
    : ScintillaEditExtern (parent)
    , d (new TextEditPrivate)
{
    QObject::connect(this, &ScintillaEditExtern::textInserted, this,
                     [=](Scintilla::Position position,
                     Scintilla::Position length, Scintilla::Position linesAdded,
                     const QByteArray &text, Scintilla::Position line){
        Q_UNUSED(position)
        Q_UNUSED(length)
        Q_UNUSED(linesAdded)
        Q_UNUSED(text)
        Q_UNUSED(line)
        emit this->fileChanged(this->file());
    }, Qt::UniqueConnection);

    QObject::connect(this, &ScintillaEditExtern::textDeleted, this,
                     [=](Scintilla::Position position,
                     Scintilla::Position length, Scintilla::Position linesAdded,
                     const QByteArray &text, Scintilla::Position line){
        Q_UNUSED(position)
        Q_UNUSED(length)
        Q_UNUSED(linesAdded)
        Q_UNUSED(text)
        Q_UNUSED(line)
        emit this->fileChanged(this->file());
    }, Qt::UniqueConnection);

    QObject::connect(this, &ScintillaEditExtern::saved, this,
                     &TextEdit::fileSaved, Qt::UniqueConnection);

    QObject::connect(DpfEventMiddleware::instance(), QOverload<const QString &, int>::of(&DpfEventMiddleware::toSearchText),
                     this, &TextEdit::find);

    QObject::connect(DpfEventMiddleware::instance(), QOverload<const QString &, const QString &, int>::of(&DpfEventMiddleware::toReplaceText),
                     this, &TextEdit::replace);
}

TextEdit::~TextEdit()
{
    emit fileClosed(file());
}

void TextEdit::setFile(const QString &filePath)
{
    ScintillaEditExtern::setFile(filePath); //顶层设置

    // 设置正则匹配规则
    if (getStyleSci()) {
        getStyleSci()->setLexer();
        getStyleSci()->setStyle();
        getStyleSci()->setMargin();
        getStyleSci()->setKeyWords();
    }
}

void TextEdit::setFile(const QString &filePath, const lsp::Head &projectHead)
{
    setFile(filePath);

    QString currFileLanguage = fileLanguage(filePath);
    if (supportLanguage() != currFileLanguage) {
        //        ContextDialog::ok(QDialog::tr("Failed, Open file language is %0, but edit support language is %1")
        //                          .arg("\"" + currFileLanguage + "\"")
        //                          .arg("\"" + supportLanguage() + "\""));
        return;
    }

    if (getStyleLsp()) {
        lsp::Client* proClient = LSPClientKeeper::instance()->get(projectHead);
        getStyleLsp()->setClient(proClient); //设置
        getStyleLsp()->initLspConnection(); // 初始化所有lsp client设置
    }
}

void TextEdit::find(const QString &srcText, int operateType)
{
    switch (operateType) {
    case FindType::Previous:
    {
        searchAnchor();
        searchPrev(SCFIND_NONE, srcText.toLatin1().data());
        break;
    }
    case FindType::Next:
    {
        findNext(srcText);
        break;
    }
    default:
        break;
    }
}

void TextEdit::replace(const QString &srcText, const QString &destText, int operateType)
{
    switch (operateType) {
    case RepalceType::Repalce:
    {
        QByteArray byteArray = getSelText();
        if (0 == QString(byteArray).compare(srcText, Qt::CaseInsensitive)) {
            replaceSel(destText.toLatin1().data());
        }

        break;
    }
    case RepalceType::FindAndReplace:
    {
        QByteArray byteArray = getSelText();
        if (0 == QString(byteArray).compare(srcText, Qt::CaseInsensitive)) {
            replaceSel(destText.toLatin1().data());
            searchAnchor();
            searchNext(SCFIND_NONE, srcText.toLatin1().data());
        }
        break;
    }
    case RepalceType::RepalceAll:
    {
        replaceAll(srcText, destText);
        break;
    }
    default:
        break;
    }
}
