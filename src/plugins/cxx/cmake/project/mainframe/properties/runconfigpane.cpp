// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

