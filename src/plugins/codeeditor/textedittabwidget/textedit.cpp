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

#include "Document.h"
#include "SciLexer.h"
#include "common/common.h"
#include "framework/framework.h"
#include "services/workspace/workspaceservice.h"

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

TextEdit::TextEdit(QWidget *parent)
    : ScintillaEditExtern (parent)
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
}

TextEdit::~TextEdit()
{
    emit fileClosed(file());
}
