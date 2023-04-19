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
#include "style/lspclientkeeper.h"
#include "Document.h"
#include "SciLexer.h"
#include "common/common.h"
#include "framework/framework.h"

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
    setFocusPolicy(Qt::ClickFocus);
    setAcceptDrops(true);
}

TextEdit::~TextEdit()
{
    // emit fileClosed(file());
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

    QString currFileLanguage = fileLanguage(filePath);
    if (supportLanguage() != currFileLanguage) {
        return;
    }

    if (getStyleLsp()) {
        // 初始化所有lsp client设置
        getStyleLsp()->initLspConnection();
    }
}

void TextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void TextEdit::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if(mimeData->hasUrls()) {
        QList<QUrl>urlList = mimeData->urls();
        QString fileName = urlList.at(0).toLocalFile();
        if(!fileName.isEmpty()) {
            editor.openFile(fileName);
        }
    }
}
