/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#include "runconfigpane.h"
#include "environmentwidget.h"

#include "services/project/projectservice.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QFormLayout>
#include <QTextBrowser>

using namespace dpfservice;

class RunConfigPanePrivate
{
    friend class RunConfigPane;

    QLineEdit *cmdArgsLineEdit{nullptr};
    QLineEdit *workingDirLineEdit{nullptr};
    QLineEdit *excutableLabel{nullptr};

    EnvironmentWidget *environmentWidget{nullptr};
    QString currentTargetName;
    config::RunParam *runParam{nullptr};
};

RunConfigPane::RunConfigPane(QWidget *parent)
    : QWidget(parent)
    , d (new RunConfigPanePrivate())
{
    setupUi();
}

RunConfigPane::~RunConfigPane()
{

}

void RunConfigPane::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    QFrame *mainFrame = new QFrame(this);
    mainFrame->setObjectName("mainframe");
    QFormLayout *formLayout = new QFormLayout(mainFrame);

    // excutable label ui.
    d->excutableLabel = new QLineEdit(mainFrame);
    d->excutableLabel->setText(tr("Here is the executable path"));
    d->excutableLabel->setReadOnly(true);
    formLayout->addRow(tr("Executable path:"), d->excutableLabel);

    // command line ui.
    d->cmdArgsLineEdit = new QLineEdit(mainFrame);
    connect(d->cmdArgsLineEdit, &QLineEdit::textChanged, [this](){
        if (d->runParam)
            d->runParam->arguments = d->cmdArgsLineEdit->text().trimmed();
    });
    formLayout->addRow(tr("Command line arguments:"), d->cmdArgsLineEdit);

    // working directory ui.
    QHBoxLayout *browLayout = new QHBoxLayout(mainFrame);
    auto browseBtn = new QPushButton(mainFrame);
    browseBtn->setText(tr("Browse..."));
    d->workingDirLineEdit = new QLineEdit(mainFrame);
    d->workingDirLineEdit->setReadOnly(true);
    connect(d->workingDirLineEdit, &QLineEdit::textChanged, [this](){
        if (d->runParam)
            d->runParam->workDirectory = d->workingDirLineEdit->text().trimmed();
    });
    browLayout->addWidget(d->workingDirLineEdit);
    browLayout->addWidget(browseBtn);
    formLayout->addRow(tr("Working directory:"), browLayout);
    connect(browseBtn, &QPushButton::clicked, [this](){
        QString outputDirectory = QFileDialog::getExistingDirectory(this, tr("Working directory"), d->workingDirLineEdit->text());
        if (!outputDirectory.isEmpty()) {
            d->workingDirLineEdit->setText(outputDirectory.toUtf8());
        }
    });

    mainFrame->setLayout(formLayout);

    d->environmentWidget = new EnvironmentWidget(this);
    vLayout->addWidget(mainFrame);
    vLayout->addWidget(d->environmentWidget);

    vLayout->setMargin(0);
}

void RunConfigPane::bindValues(config::RunParam *runParam)
{
    d->runParam = runParam;
    d->environmentWidget->bindValues(&runParam->env);
    d->cmdArgsLineEdit->setText(runParam->arguments);
    d->workingDirLineEdit->setText(runParam->workDirectory);
    d->currentTargetName = runParam->targetName;
    d->excutableLabel->setText(runParam->targetPath);
}

