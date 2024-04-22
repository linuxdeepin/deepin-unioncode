// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QWidget>

class QAbstractButton;
class ActionButtonPrivate;
class ActionButton : public QWidget
{
    Q_OBJECT
public:
    explicit ActionButton(QWidget *parent = nullptr);
    ~ActionButton();

    bool addButtons(const QStringList &list);
    QList<QAbstractButton *> buttonList();
    bool isEmpty();
    void clear();

Q_SIGNALS:
    void buttonClicked(const QString &id);

private:
    ActionButtonPrivate *const d;
};

#endif   // ACTIONBUTTON_H
