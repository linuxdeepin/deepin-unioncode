// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HOTKEYLINEEDIT_H
#define HOTKEYLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>

class HotkeyLineEditPrivate;
class HotkeyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit HotkeyLineEdit(QWidget *parent = nullptr);
    virtual ~HotkeyLineEdit() override;
    void setKey(int key);
    int getKey();
    void setText(QString text);
    void setHotkeyMode(bool hotkeyMode);
    bool isHotkeyMode();

signals:
    void sigKeyChanged(int key);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    HotkeyLineEditPrivate *const d;
};

#endif // HOTKEYLINEEDIT_H
