// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
