// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectinfodialog.h"

ProjectInfoDialog::ProjectInfoDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog (parent, f)
    , vLayout (new QVBoxLayout)
    , textBrowser (new QTextBrowser)
{
    setWindowTitle(QDialog::tr("Project Info"));
    vLayout->addWidget(textBrowser);
    setLayout(vLayout);
    setMinimumSize(450, 600);
}

void ProjectInfoDialog::setPropertyText(const QString &text)
{
    textBrowser->setText(text);
}
