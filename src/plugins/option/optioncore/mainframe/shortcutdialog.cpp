// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutdialog.h"

#include <DLineEdit>
#include <DLabel>

#include <QVBoxLayout>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

ShortcutDialog::ShortcutDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
}

void ShortcutDialog::initUI()
{
    setIcon(QIcon::fromTheme("ide"));
    setWordWrapMessage(true);
    setMessage(tr("Press desired key combination to set shortcut"));
    layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);

    keyEdit = new DLineEdit(this);
    keyEdit->installEventFilter(this);
    keyEdit->lineEdit()->installEventFilter(this);
    connect(keyEdit, &DLineEdit::textChanged, this, &ShortcutDialog::keyValueChanged);

    msgLabel = new DLabel(this);
    msgLabel->setVisible(false);
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLabel->setForegroundRole(QPalette::Highlight);
    auto f = msgLabel->font();
    f.setUnderline(true);
    msgLabel->setFont(f);

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(contentWidget);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    vLayout->addWidget(keyEdit);
    vLayout->addWidget(msgLabel);

    addContent(contentWidget);
    addButton(tr("Cancel", "button"));
    addButton(tr("Ok", "button"), true, DDialog::ButtonRecommend);
    setFocusProxy(keyEdit);
}

void ShortcutDialog::keyValueChanged()
{
    if (keyEdit->text().isEmpty())
        resetState();

    if (!checkHandler)
        return;

    int count = checkHandler(shortcutKey);
    if (count > 0) {
        QString msg = count == 1 ? tr("1 same shortcut command exist")
                                 : tr("%1 same shortcut commands exist").arg(count);
        msgLabel->setText(msg);
    }
    msgLabel->setVisible(count != 0);
}

void ShortcutDialog::resetState()
{
    keyNum = 0;
    shortcutKey = QKeySequence();
    keyEdit->clear();
    keyArray.fill(0);
}

int ShortcutDialog::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
    int result = 0;
    if ((state & Qt::ShiftModifier)
        && (text.isEmpty()
            || !text.at(0).isPrint()
            || text.at(0).isLetterOrNumber()
            || text.at(0).isSpace()))
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

QKeySequence ShortcutDialog::keySequece() const
{
    return shortcutKey;
}

void ShortcutDialog::setConflictCheckHandler(ConflictCheckHandler handler)
{
    checkHandler = handler;
}

bool ShortcutDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (keyEdit && watched != keyEdit && watched != keyEdit->lineEdit())
        return DDialog::eventFilter(watched, event);

    if (keyEdit && event->type() == QEvent::KeyPress) {
        auto k = static_cast<QKeyEvent *>(event);
        int nextKey = k->key();
        if (keyNum > 3)
            resetState();

        if (nextKey == Qt::Key_Control
            || nextKey == Qt::Key_Shift
            || nextKey == Qt::Key_Meta
            || nextKey == Qt::Key_Alt) {
            return false;
        }

        nextKey |= translateModifiers(k->modifiers(), k->text());
        switch (keyNum) {
        case 0:
            keyArray[0] = nextKey;
            break;
        case 1:
            keyArray[1] = nextKey;
            break;
        case 2:
            keyArray[2] = nextKey;
            break;
        case 3:
            keyArray[3] = nextKey;
            break;
        default:
            break;
        }
        keyNum++;
        k->accept();
        shortcutKey = QKeySequence(keyArray[0], keyArray[1], keyArray[2], keyArray[3]);
        keyEdit->setText(shortcutKey.toString(QKeySequence::NativeText));
        return true;
    }

    return DDialog::eventFilter(watched, event);
}
