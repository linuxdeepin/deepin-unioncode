// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hotkeylineedit.h"

class HotkeyLineEditPrivate
{
    HotkeyLineEditPrivate();
    bool hotkeyMode;
    int key;

    friend class HotkeyLineEdit;
};

HotkeyLineEditPrivate::HotkeyLineEditPrivate()
    : hotkeyMode(false)
    , key(Qt::Key_unknown)
{

}

HotkeyLineEdit::HotkeyLineEdit(QWidget *parent)
    : DLineEdit(parent)
    , d(new HotkeyLineEditPrivate())
{
    setAttribute(Qt::WA_InputMethodEnabled, false);
}

HotkeyLineEdit::~HotkeyLineEdit()
{

}

void HotkeyLineEdit::setHotkeyMode(bool hotkeyMode)
{
    d->hotkeyMode = hotkeyMode;
}

bool HotkeyLineEdit::isHotkeyMode()
{
    return d->hotkeyMode;
}

void HotkeyLineEdit::setKey(int key)
{
    Qt::Key qKey = static_cast<Qt::Key>(key);
    if (qKey != Qt::Key_unknown)
    {
        d->key = qKey;
        setText(QKeySequence(qKey).toString());
    }
    else
    {
        d->key = Qt::Key_unknown;
        setText("");
    }
}

int HotkeyLineEdit::getKey()
{
    return d->key;
}

void HotkeyLineEdit::setText(QString text)
{
    DLineEdit::setText(text);
}

void HotkeyLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (!d->hotkeyMode) {
        DLineEdit::keyPressEvent(e);
        return;
    }

    int key = e->key();
    Qt::Key qKey = static_cast<Qt::Key>(key);
    if (qKey == Qt::Key_unknown
        || qKey == Qt::Key_Control
        || qKey == Qt::Key_Shift
        || qKey == Qt::Key_Alt
        || qKey == Qt::Key_Enter
        || qKey == Qt::Key_Return
        || qKey == Qt::Key_Tab
        || qKey == Qt::Key_CapsLock
        || qKey == Qt::Key_Escape
        || qKey == Qt::Key_Meta)
    {
        return;
    }

    Qt::KeyboardModifiers modifiers = e->modifiers();
    if (!(modifiers & Qt::ShiftModifier
            || modifiers & Qt::ControlModifier
            || modifiers & Qt::AltModifier
            || (key >= Qt::Key_F1 && key <= Qt::Key_F35)))
    {
        return;
    }

    if (modifiers & Qt::ShiftModifier) {
        key += Qt::SHIFT;
    }

    if (modifiers & Qt::ControlModifier) {
        key += Qt::CTRL;
    }

    if (modifiers & Qt::AltModifier) {
        key += Qt::ALT;
    }

    setText(QKeySequence(key).toString());
    d->key = key;
    emit sigKeyChanged(key);
}

