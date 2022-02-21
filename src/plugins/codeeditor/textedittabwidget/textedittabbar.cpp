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
#include "textedittabbar.h"

#include <QFileInfo>
#include <QDebug>

TextEditTabBar::TextEditTabBar(QWidget *parent)
    : QTabBar (parent)
{
    setTabsClosable(true);
    setExpanding(false);

    QObject::connect(this, &QTabBar::currentChanged, this, [=](int index){
        emit this->fileSwitched(indexFile(index));
    });

    QObject::connect(this, &QTabBar::tabCloseRequested,
                     this, [=](int index) {
        this->removeTab(this->indexFile(index));
    });
}

void TextEditTabBar::setFile(const QString &file)
{
    if (!QFile::exists(file)) {
        return;
    }

    int index = fileIndex(file);
    if (index != -1) {
        setCurrentIndex(index);
        return;
    }

    QFileInfo info(file);
    int addIndex = addTab(info.fileName());
    setTabToolTip(addIndex, file);
}

void TextEditTabBar::switchFile(const QString &file)
{
    int index = fileIndex(file);
    if (index != -1) {
        setCurrentIndex(index);
    }
}

int TextEditTabBar::fileIndex(const QString &file)
{
    int index = -1;
    for (int i = 0; i < this->count(); i++) {
        if (tabToolTip(i) == file)
            index = i;
    }
    return index;
}

QString TextEditTabBar::indexFile(int index)
{
    return tabToolTip(index);
}

void TextEditTabBar::doFileChanged(const QString &file)
{
    int index = fileIndex(file);
    if (index == -1)
        return;

    QString changedFileName = "*" + QFileInfo(file).fileName();
    if (tabText(index) == changedFileName) {
        return;
    }

    setTabText(index , "*" + tabText(index));
    qInfo() << tabText(index);
}

void TextEditTabBar::doFileSaved(const QString &file)
{
    int index = fileIndex(file);
    if (index == -1)
        return;

    QString text = tabText(index);
    if (QFileInfo(file).fileName() == text){
        return;
    }

    text = text.remove(0, 1);
    setTabText(index, text);
}

void TextEditTabBar::removeTab(const QString &file)
{
    int index = fileIndex(file);
    if (index != -1){
        emit fileClosed(indexFile(index));
        QTabBar::removeTab(index);
    }
}
