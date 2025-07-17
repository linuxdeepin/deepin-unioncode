// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generatedialog.h"
#include "common/util/eventdefinitions.h"
#include "llglobal.h"

#include <DPushButton>
#include <DSuggestButton>
#include <DVerticalLine>
#include <DLineEdit>
#include <DComboBox>
#include <DRadioButton>
#include <DFileDialog>
#include <DStyle>

#include <QLabel>
#include <QFormLayout>

#include <yaml-cpp/yaml.h>
#include <fstream>

const static QString Depends_Version_V20 = "V20";
const static QString Depends_Version_V23 = "V23";

DWIDGET_USE_NAMESPACE
class GenerateDialogPrivate
{
    friend class GenerateDialog;

    DLineEdit *pathEdit { nullptr };
    DPushButton *browse { nullptr };
    DLineEdit *idEdit { nullptr };
    DLineEdit *nameEdit { nullptr };
    DLineEdit *versionEdit { nullptr };
    DComboBox *dependsCbBox { nullptr };
    DComboBox *kindCbBox { nullptr };
    DLineEdit *descriptionEdit { nullptr };
    DLineEdit *commandEdit { nullptr };
    DRadioButton *localSource { nullptr };
    DRadioButton *remoteSource { nullptr };
    QFormLayout *formLayout { nullptr };

    DDialog *newSourceDialog { nullptr };

    DPushButton *cancel { nullptr };
    DSuggestButton *confirm { nullptr };
};

GenerateDialog::GenerateDialog(QWidget *parent)
    : DDialog(parent), d(new GenerateDialogPrivate())
{
    setWindowTitle(tr("New LingLong Project"));
    setIcon(QIcon::fromTheme("ide"));

    initUi();
    initConnection();
}

void GenerateDialog::initUi()
{
    auto mainWidget = new DWidget(this);
    auto mainLayout = new QVBoxLayout(mainWidget);

    d->formLayout = new QFormLayout;
    d->formLayout->setSpacing(10);

    d->pathEdit = new DLineEdit(this);
    d->browse = new DPushButton(this);
    d->browse->setIcon(DStyle::standardIcon(style(), DStyle::SP_SelectElement));
    auto pathLayout = new QHBoxLayout;
    pathLayout->setSpacing(10);
    pathLayout->addWidget(d->pathEdit);
    pathLayout->addWidget(d->browse);

    d->idEdit = new DLineEdit(this);
    d->idEdit->setPlaceholderText("org.deepin.hello");
    d->nameEdit = new DLineEdit(this);
    d->nameEdit->setPlaceholderText(tr("Please input application`s name"));
    d->versionEdit = new DLineEdit(this);
    d->versionEdit->setPlaceholderText(tr("0.0.0.1"));
    d->kindCbBox = new DComboBox(this);
    d->kindCbBox->addItem("App");
    d->dependsCbBox = new DComboBox(this);
    d->dependsCbBox->addItems({ Depends_Version_V20, Depends_Version_V23 });
    d->descriptionEdit = new DLineEdit(this);
    d->descriptionEdit->setPlaceholderText(tr("Please input description"));
    d->commandEdit = new DLineEdit(this);
    d->commandEdit->setText("echo,-e,hello world");

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(20);
    hLayout->addWidget(d->versionEdit);
    hLayout->addWidget(new QLabel(tr("Kind:"), this));
    hLayout->addWidget(d->kindCbBox);

    auto sourceLayout = new QVBoxLayout;
    d->localSource = new DRadioButton(tr("Local\nneed to manually copy the source code to this path"), this);
    d->remoteSource = new DRadioButton(tr("Remote"), this);
    sourceLayout->addWidget(d->localSource);
    sourceLayout->addWidget(d->remoteSource);
    d->remoteSource->setChecked(true);

    d->formLayout->addRow(new QLabel(tr("ID: "), this), d->idEdit);
    d->formLayout->addRow(new QLabel(tr("Name "), this), d->nameEdit);
    d->formLayout->addRow(new QLabel(tr("Project Path: "), this), pathLayout);
    d->formLayout->addRow(new QLabel(tr("Version: "), this), hLayout);
    d->formLayout->addRow(new QLabel(tr("Description: "), this), d->descriptionEdit);
    d->formLayout->addRow(new QLabel(tr("Depends : "), this), d->dependsCbBox);
    d->formLayout->addRow(new QLabel(tr("Execute Command: "), this), d->commandEdit);
    d->formLayout->addRow(new QLabel(tr("Source Type: "), this), sourceLayout);

    mainLayout->addLayout(d->formLayout);

    auto btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 20, 0, 0);
    d->cancel = new DPushButton(tr("Cancel"), this);
    d->confirm = new DSuggestButton(tr("Confirm"), this);
    DVerticalLine *vLine = new DVerticalLine(this);
    vLine->setFixedHeight(30);
    btnLayout->addWidget(d->cancel);
    btnLayout->addWidget(vLine);
    btnLayout->addWidget(d->confirm);
    mainLayout->addLayout(btnLayout, Qt::AlignCenter);

    addContent(mainWidget);
}

void GenerateDialog::initConnection()
{
    connect(d->confirm, &DSuggestButton::clicked, this, &GenerateDialog::generate);
    connect(d->cancel, &QPushButton::clicked, this, &GenerateDialog::reject);
    connect(d->browse, &QPushButton::clicked, this, [=]() {
        QString path = DFileDialog::getExistingDirectory(this, tr("Choose path"), QDir::homePath());
        if (!path.isEmpty())
            d->pathEdit->setText(path);
    });
}

void GenerateDialog::generate()
{
    if (!checkFiledsInfo())
        return;

    YAML::Node version;
    version["version"] = "\"1\"";

    YAML::Node package;
    YAML::Node content;
    content["id"] = d->idEdit->text().toStdString();
    content["name"] = d->nameEdit->text().toStdString();
    content["version"] = d->versionEdit->text().toStdString();
    content["kind"] = d->kindCbBox->currentText().toStdString();
    content["description"] = d->descriptionEdit->text().toStdString();
    package["package"] = content;

    YAML::Node base;
    YAML::Node runtime;
    if (d->dependsCbBox->currentText() == Depends_Version_V20) {
        base["base"] = "org.deepin.foundation/20.0.0";
        runtime["runtime"] = "org.deepin.Runtime/20.0.0";
    } else {
        base["base"] = "org.deepin.foundation/23.0.0";
        runtime["runtime"] = "org.deepin.Runtime/23.0.1";
    }
    YAML::Node command;
    for (auto cmd : d->commandEdit->text().split(","))
        command["command"].push_back(cmd.toStdString());

    YAML::Node build;
    build["build"] = "echo 'hello'";

    YAML::Node sources;
    YAML::Node depend;
    depend["kind"] = "file";
    depend["url"] = "input your url";
    depend["digest"] = "f06e936eb913b8e9271c17e6d8b94d9e4f0aa558d7debdc324c9484908ee8dc8";
    YAML::Node source;
    source["kind"] = "git";
    source["url"] = "https://github.com/linuxdeepin/linglong-builder-demo.git";
    source["version"] = "master";
    source["commit"] = "a3b89c3aa34c1aff8d7f823f0f4a87d5da8d4dc0";
    sources["source"].push_back(depend);

    if (d->remoteSource->isChecked())
        sources["source"].push_back(source);

    QString path = d->pathEdit->text() + QDir::separator() + d->idEdit->text();
    QDir newDir(path);
    if (newDir.exists()) {
        DDialog dialog;
        dialog.setMessage(tr("%1 is already existed").arg(d->idEdit->text()));
        dialog.setIcon(QIcon::fromTheme("dialog-warning"));
        dialog.exec();
        return;
    }

    if (!QDir().mkdir(path)) {
        qWarning() << "Failed to create: " << path;
        return;
    }

    auto yamlFilePath = path + "/linglong.yaml";
    std::ofstream file(yamlFilePath.toStdString());
    file << "version: "
         << "\"1\""
         << "\n\n"
         << package << "\n\n"
         << base << "\n"
         << "#" << runtime << "\n\n"
         << command << "\n\n"
         << "#set the sources if you need, a simple example as follows\n"
         << sources << "\n\n"
         << build << "  #some operation to build this project";
    file.close();

    project.openProject(LL_TOOLKIT, LL_LANGUAGE, path);
    accept();
}

bool GenerateDialog::checkFiledsInfo()
{
    QList<DLineEdit *> editList { d->idEdit, d->nameEdit, d->versionEdit, d->descriptionEdit, d->commandEdit };
    bool ret = true;
    for (auto edit : editList) {
        if (edit->text().isEmpty()) {
            edit->setPlaceholderText(tr("This item can not be empty"));
            ret = false;
        }
    }

    DDialog dialog;
    dialog.setIcon(QIcon::fromTheme("dialog-warning"));
    auto path = d->pathEdit->text();
    QDir dir(path);
    if (path.isEmpty() || !dir.exists()) {
        d->pathEdit->setText("");
        dialog.setMessage(tr("Project Path is empty or not exist"));
        dialog.exec();
        ret = false;
        return ret;
    }

    QRegularExpression regex("^(\\d+\\.){3}\\d+$");
    auto match = regex.match(d->versionEdit->text());
    if (!match.hasMatch()) {
        dialog.setMessage(tr("Version must be in the format of *.*.*.*"));
        ret = false;
        dialog.exec();
    }

    return ret;
}
