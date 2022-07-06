/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "adapterconfigure.h"

AdapterConfigure::AdapterConfigure(QWidget *parent)
    : PageWidget (parent)
    , checkBoxProc(new QCheckBox(QCheckBox::tr("Read from Stdout:"), this))
    , checkBoxSock(new QCheckBox(QCheckBox::tr("Read from Socket:"), this))
    , mainVLayout(new QVBoxLayout(this))
    , checkBoxGroup(new QButtonGroup(this))
    , sections({{checkBoxProc, new ProcConfigure(this)},
                {checkBoxSock, new SockConfigure(this)}})
{
    checkBoxGroup->addButton(checkBoxProc);
    checkBoxGroup->addButton(checkBoxSock);
    mainVLayout->addWidget(checkBoxProc);
    mainVLayout->setSpacing(0);
    mainVLayout->addWidget(sections[checkBoxProc]);
    mainVLayout->setSpacing(0);
    mainVLayout->addWidget(checkBoxSock);
    mainVLayout->setSpacing(0);
    mainVLayout->addWidget(sections[checkBoxSock]);
    setLayout(mainVLayout);
    QObject::connect(checkBoxProc, &QCheckBox::clicked,
                     this, &AdapterConfigure::clicked);
    QObject::connect(checkBoxSock, &QCheckBox::clicked,
                     this, &AdapterConfigure::clicked);
    checkBoxProc->setChecked(true);
    checkBoxProc->clicked();
}

void AdapterConfigure::clicked()
{
    QCheckBox *checkBox = qobject_cast<QCheckBox*> (sender());
    if (!checkBox)
        return;
    auto itera = sections.begin();
    while (itera != sections.end()) {
        if (checkBox == itera.key()) {
            itera.value()->setEnabled(true);
        } else {
            itera.value()->setEnabled(false);
        }
        itera ++;
    }
}

SockConfigure::SockConfigure(QWidget *parent)
    : ProcConfigure(parent)
    , ip (new QLineEdit(this))
    , port (new QLineEdit(this))
{
    ip->setPlaceholderText(QLineEdit::tr("listen backend ip"));
    port->setPlaceholderText(QLineEdit::tr("listen backend port"));
    vLayout->addWidget(ip);
    vLayout->addWidget(port);
}

ProcConfigure::ProcConfigure(QWidget *parent)
    : QWidget(parent)
    , lauchCmd(new QLineEdit(this))
    , vLayout(new QVBoxLayout(this))
{
    lauchCmd->setPlaceholderText(
                QLineEdit::tr("launch program command"));
    vLayout->addWidget(lauchCmd);
    setLayout(vLayout);
}
