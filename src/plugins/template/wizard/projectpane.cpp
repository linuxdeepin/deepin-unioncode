// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectpane.h"

#include <QFormLayout>

#include <DLineEdit>
#include <DComboBox>
#include <DLabel>
#include <DSuggestButton>
#include <DFileDialog>

DWIDGET_USE_NAMESPACE
ProjectPane::ProjectPane(const QString &title, DWidget *parent)
    : AbstractPane(parent), shortTitle(title)
{
    setupUi();
}

void ProjectPane::setupUi()
{
    DLabel *title = new DLabel(this);
    title->setText(shortTitle);

    QFormLayout *mainLayout = new QFormLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    mainLayout->addRow(title);

    DLabel *project = new DLabel(tr("project Name :"), this);
    projectEdit = new DLineEdit(this);
    mainLayout->addRow(project, projectEdit);

    DLabel *pathChooser = new DLabel(tr("Location :"), this);
    locationEdit = new DLineEdit(this);
    locationEdit->lineEdit()->setReadOnly(true);

    DSuggestButton *browse = new DSuggestButton("...", this);
    browse->setFixedSize(36, 36);

    connect(browse, &DSuggestButton::clicked, [=]() {
        QString path = DFileDialog::getExistingDirectory(this, tr("Choose path"), QDir::homePath());
        if (!path.isEmpty())
            locationEdit->setText(path);
    });

    auto hLayout = new QHBoxLayout();
    hLayout->addWidget(locationEdit);
    hLayout->addWidget(browse);

    mainLayout->addRow(pathChooser, hLayout);
}

QMap<QString, QVariant> ProjectPane::getValue()
{
    value.insert("ProjectName", projectEdit->text());
    value.insert("Location", locationEdit->text());
    return value;
}
