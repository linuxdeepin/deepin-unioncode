/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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

#include "binarytoolsconfigview.h"
#include "binarytoolssetting.h"
#include "environmentview.h"

#include "common/widget/collapsewidget.h"

#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QFont>
#include <QInputDialog>
#include <qmessagebox.h>
#include <QGridLayout>
#include <QLabel>
#include <QStandardPaths>
#include <QScrollArea>

static QString CURRENT_COMMAND = "Current command";
static QString ALL_COMMAND = "All command";
static QString ENVIRONMENT = "Environment";

class BinaryToolsConfigViewPrivate
{
    friend class BinaryToolsConfigView;

    QGridLayout *gridLayout = nullptr;
    QWidget *compatConfigWidget = nullptr;
    QComboBox *runComandCombo = nullptr;
    QLineEdit *toolArgsEdit = nullptr;
    QLabel *nameLabel = nullptr;
    QLabel *commandLabel = nullptr;
    QLineEdit *executableDirEdit = nullptr;
    QLineEdit *workingDirEdit = nullptr;
    EnvironmentView *envView = nullptr;
    QPushButton *addButton = nullptr;
    QPushButton *deleteButton = nullptr;
    QPushButton *renameButton = nullptr;
};

BinaryToolsConfigView::BinaryToolsConfigView(QWidget *parent)
    : QWidget(parent)
    , d(new BinaryToolsConfigViewPrivate)
{
    d->compatConfigWidget = new QWidget(this);
    d->nameLabel = new QLabel();
    d->runComandCombo = new QComboBox(this);
    d->runComandCombo->setMinimumContentsLength(15);
    d->runComandCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    d->addButton = new QPushButton(tr("Add"), this);
    d->deleteButton = new QPushButton(tr("Delete"), this);
    d->renameButton = new QPushButton(tr("Rename"), this);

    d->gridLayout = new QGridLayout(this);
    d->gridLayout->setSpacing(6);
    d->gridLayout->setContentsMargins(10, 10, 10, 10);

    auto configLabel = new QLabel(this);
    configLabel->setText(tr("Binary configuration:"));
    auto spacer1 = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto spacer2 = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);

    d->gridLayout->addWidget(configLabel, 0, 0, 1, 1);
    d->gridLayout->addWidget(d->runComandCombo, 0, 1, 1, 1);
    d->gridLayout->addWidget(d->addButton, 0, 2, 1, 1);
    d->gridLayout->addWidget(d->deleteButton, 0, 3, 1, 1);
    d->gridLayout->addWidget(d->renameButton, 0, 4, 1, 1);
    d->gridLayout->addItem(spacer1, 0, 5, 1, 1);
    d->gridLayout->addWidget(d->compatConfigWidget, 1, 0, 1, 5);
    d->gridLayout->addItem(spacer2, 2, 0, 1, 1);

    setConfigWidget();
    readConfig();

    connect(d->runComandCombo, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &BinaryToolsConfigView::currentConfigChanged);

    connect(d->addButton, &QPushButton::clicked,
            this, &BinaryToolsConfigView::addCompatConfig);

    connect(d->deleteButton, &QPushButton::clicked,
            this, &BinaryToolsConfigView::deleteCompatConfig);

    connect(d->renameButton, &QPushButton::clicked,
            this, &BinaryToolsConfigView::renameCompatConfig);
}

BinaryToolsConfigView::~BinaryToolsConfigView()
{
    if (d)
        delete d;
}

void BinaryToolsConfigView::initializeUi()
{
    d->runComandCombo->addItem("ls");
    d->runComandCombo->addItem("ps");
    d->runComandCombo->setCurrentIndex(0);

    BinaryToolsSetting settings;
    settings.setValue(CURRENT_COMMAND, d->runComandCombo->currentText());
    settings.setValue(ALL_COMMAND, QStringList() << "ls" << "ps");
    settings.setValue("ls", QStringList() << "/usr/bin/ls" << d->toolArgsEdit->text());
    settings.setValue("ps", QStringList() << "/usr/bin/ps" << d->toolArgsEdit->text());
}

bool BinaryToolsConfigView::saveConfig()
{
    QString curCommand = d->runComandCombo->currentText();
    if (curCommand.isEmpty())
        return false;

    if (d->executableDirEdit->text().isEmpty()) {
        QMessageBox msgBox(QMessageBox::Warning, tr("Warning"),
                           tr("Please select working directory."),
                           QMessageBox::Ok, d->compatConfigWidget);
        msgBox.exec();
        return false;
    }

    if (d->toolArgsEdit->text().isEmpty())
        d->toolArgsEdit->setText("");
    QStringList commandList = QStringList() << d->executableDirEdit->text() << d->toolArgsEdit->text()
                                            << d->nameLabel->text() << d->workingDirEdit->text();
    BinaryToolsSetting settings;
    settings.setValue(CURRENT_COMMAND, curCommand);
    settings.setValue(curCommand, commandList);
    settings.setValue(curCommand + ENVIRONMENT, d->envView->getEnvironment());
    return true;
}

void BinaryToolsConfigView::readConfig()
{
    BinaryToolsSetting settings;
    QStringList allCommand = qvariant_cast<QStringList>(settings.getValue(ALL_COMMAND));
    for (QString command : allCommand) {
        if (d->runComandCombo->findText(command) == -1)
            d->runComandCombo->addItem(command);
    }

    QString curCommand = qvariant_cast<QString>(settings.getValue(CURRENT_COMMAND, ""));
    if (curCommand.isEmpty())
        return;
    int index = d->runComandCombo->findText(curCommand);
    d->runComandCombo->setCurrentIndex(index);
    updateView(curCommand);
}

QString BinaryToolsConfigView::getProgram()
{
    saveConfig();
    return d->executableDirEdit->text();
}

QStringList BinaryToolsConfigView::getArguments()
{
    return d->toolArgsEdit->text().split(" ");
}

QString BinaryToolsConfigView::getWorkingDir()
{
    return d->workingDirEdit->text();
}

QMap<QString, QVariant> BinaryToolsConfigView::getEnvironment()
{
    return d->envView->getEnvironment();
}

void BinaryToolsConfigView::updateView(const QString &command)
{
    if (command.isEmpty()) {
        d->nameLabel->setText("");
        d->commandLabel->setText("");
        d->toolArgsEdit->setText("");
        d->executableDirEdit->setText("");
        d->workingDirEdit->setText("");
        d->envView->setValue({});
        return;
    }

    BinaryToolsSetting settings;
    QStringList argList = qvariant_cast<QStringList>(settings.getValue(command, "/usr/bin/" + command));
    if (argList.isEmpty())
        return;
    d->executableDirEdit->setText(argList.at(0));
    d->toolArgsEdit->setText(argList.size() > 1 ? argList.at(1) : "");
    d->nameLabel->setText(argList.size() > 2 ? argList.at(2) : "");
    d->workingDirEdit->setText(argList.size() > 3 ? argList.at(3) : "");
    d->commandLabel->setText(d->executableDirEdit->text() + " " + d->toolArgsEdit->text());

    QMap<QString, QVariant> map = qvariant_cast<QMap<QString, QVariant>>(settings.getValue(command + ENVIRONMENT));
    d->envView->setValue(map);
}

void BinaryToolsConfigView::currentConfigChanged(const QString &text)
{
    BinaryToolsSetting settings;
    if (settings.getValue(CURRENT_COMMAND).toString() == text)
        return;

    settings.setValue(CURRENT_COMMAND, text);
    updateView(text);
    emit comboChanged();
}

void BinaryToolsConfigView::addCompatConfig()
{
    if (!d->runComandCombo->currentText().isEmpty() && !saveConfig())
        return;

    bool ok;
    QString name = QInputDialog::getText(this, tr("Add new command"),
                                         tr("New command name:"),
                                         QLineEdit::Normal, "", &ok);
    if (!ok)
        return;

    QString uniName = uniqueName(name);
    if (uniName.isEmpty())
        return;

    BinaryToolsSetting settings;
    QStringList allCommand = qvariant_cast<QStringList>(settings.getValue(ALL_COMMAND));
    allCommand.append(uniName);
    settings.setValue(CURRENT_COMMAND, uniName);
    settings.setValue(ALL_COMMAND, allCommand);

    d->nameLabel->setText(name);
    d->toolArgsEdit->setText("");
    d->executableDirEdit->setText("/usr/bin/" + name);
    d->workingDirEdit->setText(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    d->envView->initModel();
    QStringList commandList = QStringList() << d->executableDirEdit->text()<< d->toolArgsEdit->text()
                                            << d->nameLabel->text() << d->workingDirEdit->text();
    settings.setValue(uniName, commandList);
    settings.setValue(uniName + ENVIRONMENT, d->envView->getEnvironment());

    //call currentConfigChanged()
    d->runComandCombo->addItem(uniName);
    d->runComandCombo->setCurrentText(uniName);
    d->deleteButton->setEnabled(true);
}

void BinaryToolsConfigView::deleteCompatConfig()
{
    QMessageBox msgBox(QMessageBox::Question, tr("Delete Configuration?"),
                       tr("Do you really want to delete the command <b>%1</b>?").arg(d->nameLabel->text()),
                       QMessageBox::Yes|QMessageBox::No, this);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setEscapeButton(QMessageBox::No);
    if (msgBox.exec() == QMessageBox::No)
        return;

    BinaryToolsSetting settings;
    QStringList allCommand = qvariant_cast<QStringList>(settings.getValue(ALL_COMMAND));
    allCommand.removeOne(d->runComandCombo->currentText());
    settings.setValue(ALL_COMMAND, allCommand);
    settings.deleteKey(d->runComandCombo->currentText());
    settings.deleteKey(d->runComandCombo->currentText() + ENVIRONMENT);

    int curIndex = d->runComandCombo->currentIndex();
    d->runComandCombo->removeItem(curIndex);
    d->runComandCombo->setCurrentIndex((curIndex - 1) >= 0 ? curIndex - 1 : 0);
    if (d->runComandCombo->currentText().isEmpty())
        d->deleteButton->setEnabled(false);
}

void BinaryToolsConfigView::renameCompatConfig()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Rename..."),
                                         tr("New name for command <b>%1</b>:").
                                         arg(d->runComandCombo->currentText()),
                                         QLineEdit::Normal,
                                         d->runComandCombo->currentText(), &ok);
    if (!ok)
        return;

    if (name == d->runComandCombo->currentText())
        return;

    QString uniName = uniqueName(name);
    if (uniName.isEmpty())
        return;

    BinaryToolsSetting settings;
    QStringList allCommand = qvariant_cast<QStringList>(settings.getValue(ALL_COMMAND));
    int index = allCommand.indexOf(d->runComandCombo->currentText());
    if (index != -1) {
        allCommand.replace(index, uniName);
    } else {
        allCommand.append(uniName);
    }
    settings.setValue(CURRENT_COMMAND, uniName);
    settings.setValue(ALL_COMMAND, allCommand);
    d->nameLabel->setText(name);
    QStringList commandList = QStringList() << d->executableDirEdit->text()<< d->toolArgsEdit->text()
                                            << d->nameLabel->text() << d->workingDirEdit->text();
    settings.setValue(uniName, commandList);
    settings.setValue(uniName + ENVIRONMENT, d->envView->getEnvironment());
    settings.deleteKey(d->runComandCombo->currentText());
    settings.deleteKey(d->runComandCombo->currentText() + ENVIRONMENT);

    int itemIndex = d->runComandCombo->currentIndex();
    d->runComandCombo->insertItem(itemIndex + 1, uniName);
    d->runComandCombo->setCurrentText(uniName);
    d->runComandCombo->removeItem(itemIndex);
}

void BinaryToolsConfigView::setConfigWidget()
{
    d->compatConfigWidget->setContentsMargins(0, 0, 0, 0);
    d->compatConfigWidget->setAutoFillBackground(true);
    auto cmdLabel = new QLabel(d->compatConfigWidget);
    QFont ft;
    ft.setBold(true);
    cmdLabel->setFont(ft);
    cmdLabel->setText(tr("Command:"));
    d->commandLabel = new QLabel(d->compatConfigWidget);

    auto argsLabel = new QLabel(d->compatConfigWidget);
    argsLabel->setText(tr("Tool arguments:"));
    d->toolArgsEdit = new QLineEdit(d->compatConfigWidget);
    d->toolArgsEdit->setPlaceholderText(tr("Input your arguments."));

    auto exeLabel = new QLabel(d->compatConfigWidget);
    exeLabel->setText(tr("Executable:"));
    d->executableDirEdit = new QLineEdit(d->compatConfigWidget);
    auto browseButton1 = new QPushButton(tr("Browse..."), d->compatConfigWidget);

    auto workLabel = new QLabel(d->compatConfigWidget);
    workLabel->setText(tr("Working directory:"));
    d->workingDirEdit = new QLineEdit(d->compatConfigWidget);
    auto browseButton2 = new QPushButton(tr("Browse..."), d->compatConfigWidget);

    auto envLabel = new QLabel(d->compatConfigWidget);
    envLabel->setText(tr("Envrioment: Base environment for this command configuration."));
    auto appendButton = new QPushButton(tr("&Append"), d->compatConfigWidget);
    auto deleteButton = new QPushButton(tr("&Delete"), d->compatConfigWidget);
    auto resetButton = new QPushButton(tr("&Reset"), d->compatConfigWidget);
    deleteButton->setEnabled(false);
    d->envView = new EnvironmentView();
    d->envView->setFixedHeight(250);

    auto gridLayout = new QGridLayout(d->compatConfigWidget);
    gridLayout->setSpacing(5);
    gridLayout->setContentsMargins(5, 5, 5, 5);

    gridLayout->addWidget(cmdLabel, 0, 0, 1, 1);
    gridLayout->addWidget(d->commandLabel, 0, 1, 1, 2);
    gridLayout->addWidget(argsLabel, 1, 0, 1, 1);
    gridLayout->addWidget(d->toolArgsEdit, 1, 1, 1, 2);
    gridLayout->addWidget(exeLabel, 2, 0, 1, 1);
    gridLayout->addWidget(d->executableDirEdit, 2, 1, 1, 1);
    gridLayout->addWidget(browseButton1, 2, 2, 1, 1);
    gridLayout->addWidget(workLabel, 3, 0, 1, 1);
    gridLayout->addWidget(d->workingDirEdit, 3, 1, 1, 1);
    gridLayout->addWidget(browseButton2, 3, 2, 1, 1);
    gridLayout->addWidget(envLabel, 4, 0, 1, 3);
    gridLayout->addWidget(appendButton, 5, 0, 1, 1);
    gridLayout->addWidget(deleteButton, 6, 0, 1, 1);
    gridLayout->addWidget(resetButton, 7, 0, 1, 1);
    gridLayout->addWidget(d->envView, 5, 1, 5, 2);
    d->compatConfigWidget->setLayout(gridLayout);

    connect(browseButton1, &QPushButton::clicked, [=]() {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        QString filePath = QFileDialog::getOpenFileName(nullptr, tr("Select Executabel Path"), dir);
        if (filePath.isEmpty() && !QFileInfo(filePath).exists())
            return;
        d->executableDirEdit->setText(filePath);
    });

    connect(browseButton2, &QPushButton::clicked, [=]() {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        QString filePath = QFileDialog::getExistingDirectory(nullptr, tr("Select Working Directory"), dir);
        if (filePath.isEmpty() && !QFileInfo(filePath).exists())
            return;
        d->workingDirEdit->setText(filePath);
    });

    connect(appendButton, &QPushButton::clicked, d->envView, &EnvironmentView::appendRow);
    connect(deleteButton, &QPushButton::clicked, d->envView, &EnvironmentView::deleteRow);
    connect(resetButton, &QPushButton::clicked, d->envView, &EnvironmentView::initModel);

    connect(d->toolArgsEdit, &QLineEdit::textChanged, [=](){
        d->commandLabel->setText(d->executableDirEdit->text() + " " + d->toolArgsEdit->text());
    });

    connect(d->executableDirEdit, &QLineEdit::textChanged, [=](){
        d->commandLabel->setText(d->executableDirEdit->text() + " " + d->toolArgsEdit->text());
    });

    connect(d->envView, &EnvironmentView::deleteSignal, [=](bool enable){
       deleteButton->setEnabled(enable);
    });

    connect(this, &BinaryToolsConfigView::comboChanged, [=]{
        deleteButton->setEnabled(false);
    });
}

QString BinaryToolsConfigView::uniqueName(const QString &name)
{
    QString result = name.trimmed();
    if (!result.isEmpty()) {
        if (d->runComandCombo->findText(result) == -1)
            return result;
        int i = 2;

        QString tryName = result + QString::number(i);
        while (d->runComandCombo->findText(tryName) != -1) {
            tryName = result + QString::number(++i);
        }
        return tryName;
    }
    return result;
}
