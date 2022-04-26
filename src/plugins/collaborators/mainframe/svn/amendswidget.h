/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#ifndef AMENDSWIDGET_H
#define AMENDSWIDGET_H

#include "common/common.h"

#include <QSplitter>
#include <QLineEdit>
#include <QTextEdit>

class QVBoxLayout;
class QStackedWidget;
class QPushButton;
class QLineEdit;
class QHBoxLayout;
class QVBoxLayout;
class FileModifyView;
class FileSourceView;

class AmendsWidget : public QSplitter
{
    Q_OBJECT
public:
    explicit AmendsWidget(QWidget* parent = nullptr);
    virtual ~AmendsWidget();
    QString description();
    FileModifyView *modView();

signals:
    void commitClicked();
    void revertAllClicked();

private:
    FileModifyView *modifyView{nullptr};
    QHBoxLayout *hLayPbt{nullptr};
    QPushButton *pbtCommit{nullptr};
    QPushButton *pbtRevertAll{nullptr};
    QTextEdit *descEdit{nullptr};
    QFrame *pbtWidget{nullptr};
};

#endif // AMENDSWIDGET_H
