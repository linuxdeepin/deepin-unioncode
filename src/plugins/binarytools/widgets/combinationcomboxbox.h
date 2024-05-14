// SPDX-FileCopyrigh tText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMBINATIONCOMBOXBOX_H
#define COMBINATIONCOMBOXBOX_H

#include "constants.h"

#include <QComboBox>

class CompleterViewPrivate;
class CompleterView : public QFrame
{
    Q_OBJECT
public:
    explicit CompleterView(QWidget *parent = nullptr);
    ~CompleterView();

    void updateTools(const QMap<QString, QList<ToolInfo> > &tools);

protected:
    void showEvent(QShowEvent *e) override;

private:
    CompleterViewPrivate *const d;
};

class CombinationComboxBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CombinationComboxBox(QWidget *parent = nullptr);

    void setTools(const QMap<QString, QList<ToolInfo>> &tools);
    void setText(const QString &text);
    QString text();
    void clear();

    void showPopup() override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    CompleterView *completerView { nullptr };
};

#endif   // COMBINATIONCOMBOXBOX_H
