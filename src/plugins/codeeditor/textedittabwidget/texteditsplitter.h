/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
 *
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
#ifndef TEXTEDITSPLITTER_H
#define TEXTEDITSPLITTER_H

#include "textedittabwidget.h"

#include <QObject>
#include <QGridLayout>
#include <QSplitter>
#include <QVector>
#include <QList>

class TextEditSplitter : public QWidget
{
    Q_OBJECT
public:
    friend class TextEditTabWidget;

    explicit TextEditSplitter(QWidget *parent = nullptr);
    QSplitter *getSplitter() const;
    void editSplit(Qt::Orientation orientation, const QString &file);
    void closeSplit();
    void doSelectedTextEditWidget();
    static TextEditSplitter *instance();
private:
    virtual ~TextEditSplitter() override;
    QVBoxLayout *vLayout = nullptr;
    QSplitter *mainSplitter = nullptr;
    TextEditTabWidget *tabWidget = nullptr;
    QList<TextEditTabWidget *> *tabWidgets = nullptr;
    QList<QSplitter *> newSplitters;
    int count = 0;
};

#endif // TEXTEDITSPLITTER_H
