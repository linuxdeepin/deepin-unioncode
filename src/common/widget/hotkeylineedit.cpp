/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

#include "hotkeylineedit.h"

HotkeyLineEdit::HotkeyLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_nKey(Qt::Key_unknown)
    , m_bHotkeyMode(false)
{

}

HotkeyLineEdit::~HotkeyLineEdit()
{

}

void HotkeyLineEdit::setHotkeyMode(bool bHotkeyMode)
{
    m_bHotkeyMode = bHotkeyMode;
}

bool HotkeyLineEdit::isHotkeyMode()
{
    return m_bHotkeyMode;
}

void HotkeyLineEdit::setKey(int nKey)
{
    Qt::Key qKey = static_cast<Qt::Key>(nKey);
    if (qKey != Qt::Key_unknown)
    {
        m_nKey = qKey;
        setText(QKeySequence(qKey).toString());
    }
    else
    {
        m_nKey = Qt::Key_unknown;
        setText("");
    }
}

int HotkeyLineEdit::getKey()
{
    return m_nKey;
}

void HotkeyLineEdit::setText(QString qsText)
{
    QLineEdit::setText(qsText);
}

void HotkeyLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (!m_bHotkeyMode) {
        QLineEdit::keyPressEvent(e);
        return;
    }

    int nKey = e->key();
    Qt::Key qKey = static_cast<Qt::Key>(nKey);
    if (qKey == Qt::Key_unknown)
    {
        return;
    }

    if (qKey == Qt::Key_Control
        || qKey == Qt::Key_Shift
        || qKey == Qt::Key_Alt
        || qKey == Qt::Key_Enter
        || qKey == Qt::Key_Return
        || qKey == Qt::Key_Tab
        || qKey == Qt::Key_CapsLock
        || qKey == Qt::Key_Escape)
    {
        return;
    }

    if (qKey == Qt::Key_Backspace)
    {
        emit sigKeyChanged(Qt::Key_unknown);
        m_nKey = Qt::Key_unknown;
        setText("");
        return;
    }

    Qt::KeyboardModifiers modifiers = e->modifiers();
    if (!(modifiers & Qt::ShiftModifier
            || modifiers & Qt::ControlModifier
            || modifiers & Qt::AltModifier
            || (e->key() >= Qt::Key_F1 && e->key() <= Qt::Key_F35)))
    {
        return;
    }

    if (modifiers & Qt::ShiftModifier)
    {
        nKey += Qt::SHIFT;
    }

    if (modifiers & Qt::ControlModifier)
    {
        nKey += Qt::CTRL;
    }

    if (modifiers & Qt::AltModifier)
    {
        nKey += Qt::ALT;
    }

    setText(QKeySequence(nKey).toString());
    m_nKey = nKey;
    emit sigKeyChanged(nKey);
}
