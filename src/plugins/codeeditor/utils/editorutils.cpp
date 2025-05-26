// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorutils.h"

#include "common/actionmanager/actionmanager.h"

#include <QFile>
#include <QTextStream>

int EditorUtils::nbDigitsFromNbLines(long nbLines)
{
    int nbDigits = 0;   // minimum number of digit should be 4
    if (nbLines < 10)
        nbDigits = 1;
    else if (nbLines < 100)
        nbDigits = 2;
    else if (nbLines < 1000)
        nbDigits = 3;
    else if (nbLines < 10000)
        nbDigits = 4;
    else if (nbLines < 100000)
        nbDigits = 5;
    else if (nbLines < 1000000)
        nbDigits = 6;
    else {   // rare case
        nbDigits = 7;
        nbLines /= 1000000;

        while (nbLines) {
            nbLines /= 10;
            ++nbDigits;
        }
    }

    return nbDigits;
}

Command *EditorUtils::registerShortcut(QAction *act, const QString &id, const QKeySequence &shortCut)
{
    auto cmd = ActionManager::instance()->registerAction(act, id);
    cmd->setDefaultKeySequence(shortCut);
    return cmd;
}

QVector<QPair<QString, QStringList>> EditorUtils::supportEncoding()
{
    static QVector<QPair<QString, QStringList>> groupEncodeVec;
    if (groupEncodeVec.isEmpty()) {
        QFile file(":/encodes/encodes.ini");
        QString data;
        if (file.open(QIODevice::ReadOnly)) {
            data = QString::fromUtf8(file.readAll());
            file.close();
        }

        QTextStream readStream(&data, QIODevice::ReadOnly);
        while (!readStream.atEnd()) {
            QString group = readStream.readLine();
            QString key = group.mid(1, group.length() - 2);
            QString encodes = readStream.readLine();
            QString value = encodes.mid(8, encodes.length() - 2);
            groupEncodeVec.append(QPair<QString, QStringList>(key, value.split(",")));
        }
    }

    return groupEncodeVec;
}
