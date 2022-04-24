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

#include <QFrame>
#include <QLineEdit>
#include <QTextEdit>

class QVBoxLayout;
class QStackedWidget;
class QPushButton;
class QLineEdit;
class QHBoxLayout;
class QVBoxLayout;
class FileModifyWidget;

extern const QString Description;
extern const QString Summary;
extern const QString Commit;
extern const QString Cancel;

class AmendsWidget : public QFrame
{
    Q_OBJECT
public:
    explicit AmendsWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~AmendsWidget();
    void reflashAmends(const QSet<QString> &keys = {Description});
    QHash<QString, QString> amendValues();
    QString amendValue(const QString &key);
    FileModifyWidget *fileAddedWidget();
    FileModifyWidget *fileSourceWidget();

signals:
    void commitClicked();
    void revertAllClicked();

private:
    FileModifyWidget *fListAdded{nullptr};
    FileModifyWidget *fListSource{nullptr};
    QVBoxLayout *vLayoutAmend{nullptr};
    QHBoxLayout *hLayPbt{nullptr};
    QPushButton *pbtCommit{nullptr};
    QPushButton *pbtRevertAll{nullptr};
    QVBoxLayout *vLayoutMain{nullptr};
};

#endif // AMENDSWIDGET_H
