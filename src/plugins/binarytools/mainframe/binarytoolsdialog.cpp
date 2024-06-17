// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytoolsdialog.h"
#include "binarytoolsconfigview.h"

#include <DPushButton>
#include <DSuggestButton>
#include <DLabel>
#include <DFrame>
#include <DIconTheme>

#include <QIcon>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
class BinaryToolsDialogPrivate
{
    friend class BinaryToolsDialog;
    BinaryToolsConfigView *configView = nullptr;
};

BinaryToolsDialog::BinaryToolsDialog(QDialog *parent)
    : DDialog(parent), d(new BinaryToolsDialogPrivate)
{
    setWindowTitle(tr("Binary Tools"));
    setFixedSize(730, 677);
    setIcon(DIconTheme::findQIcon("ide"));

    DWidget *mainFrame = new DWidget(this);
    addContent(mainFrame);
    QVBoxLayout *vLayout = new QVBoxLayout(mainFrame);
    d->configView = new BinaryToolsConfigView(mainFrame);

    DLabel *configLabel = new DLabel(tr("Running Configuration:"), this);
    vLayout->addWidget(configLabel);
    vLayout->addWidget(d->configView);

    QHBoxLayout *buttonLayout = new QHBoxLayout(mainFrame);
    DPushButton *cancelButton = new DPushButton(tr("Cancel", "button"));
    cancelButton->setFixedWidth(173);
    DSuggestButton *okButton = new DSuggestButton(tr("Ok", "button"));
    okButton->setFixedWidth(173);

    DVerticalLine *lLine = new DVerticalLine;
    lLine->setObjectName("VLine");
    lLine->setFixedHeight(30);

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(lLine);
    buttonLayout->addWidget(okButton);
    buttonLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    vLayout->addLayout(buttonLayout);

    connect(okButton, &DPushButton::clicked, this, &BinaryToolsDialog::saveClicked);
    connect(cancelButton, &DPushButton::clicked, this, &BinaryToolsDialog::close);
}

BinaryToolsDialog::~BinaryToolsDialog()
{
    if (d)
        delete d;
}

void BinaryToolsDialog::saveClicked()
{
    d->configView->saveConfig();
    close();
}

