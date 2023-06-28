// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
