// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AMENDSWIDGET_H
#define AMENDSWIDGET_H

#include "common/common.h"

#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DTextEdit>
#include <DSplitter>
#include <DFrame>

class QVBoxLayout;
class QHBoxLayout;
class QVBoxLayout;
class FileModifyView;
class FileSourceView;

DWIDGET_USE_NAMESPACE
class AmendsWidget : public DSplitter
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
    DPushButton *pbtCommit{nullptr};
    DPushButton *pbtRevertAll{nullptr};
    DTextEdit *descEdit{nullptr};
    DFrame *pbtWidget{nullptr};
};

#endif // AMENDSWIDGET_H
