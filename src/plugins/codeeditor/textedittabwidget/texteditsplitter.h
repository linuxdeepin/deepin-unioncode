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
#include <QList>
#include <QPair>

class TextEditSplitter : public QWidget
{
    Q_OBJECT
public:
    friend class TextEditTabWidget;

    explicit TextEditSplitter(QWidget *parent = nullptr);
    QSplitter *getSplitter() const;

public slots:
    void doSplit(Qt::Orientation orientation, const newlsp::ProjectKey &key, const QString &file);
    void doClose();
    void doSelected(bool state);
    void doShowSplit();
    static TextEditSplitter *instance();

private:
    virtual ~TextEditSplitter() override;
    void updateSplitter(QSplitter *splitter, TextEditTabWidget *textEditTabWidget);
    QVBoxLayout *vLayout = nullptr;
    QSplitter *mainSplitter = nullptr;
    TextEditTabWidget *tabWidget = nullptr;
    QHash<TextEditTabWidget *, bool> tabWidgets;
    QHash<QSplitter *, QPair<TextEditTabWidget *, TextEditTabWidget*>> splitters;
};

#endif // TEXTEDITSPLITTER_H
