// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DKeySequenceEdit>
#include <DMenu>

#include <QAction>
#include <QContextMenuEvent>

using Dtk::Widget::DMenu;

class ShortCutEdit : public Dtk::Widget::DKeySequenceEdit
{
    Q_OBJECT
public:
    ShortCutEdit(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override 
    {
        menu->exec(event->globalPos());
    }

signals:
    void shortcutCleared();

private slots:
    void clearText();

private:
    DMenu *menu = nullptr;
};

