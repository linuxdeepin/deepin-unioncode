// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"

#include "services/option/optionmanager.h"

#include <DLabel>
#include <DFrame>
#include <DLineEdit>

#include <QVBoxLayout>
#include <QEvent>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

SettingDialog::SettingDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnection();
    updateSettings();
}

void SettingDialog::initUI()
{
    setFixedSize(550, 600);
    setIcon(QIcon::fromTheme("ide"));
    setWindowTitle(tr("Settings"));
    setOnButtonClickedClose(false);

    auto createButton = [this](const QIcon &icon, const QString &tips) {
        auto btn = new QPushButton(this);
        btn->setIconSize({ 16, 16 });
        btn->setIcon(icon);
        btn->setToolTip(tips);
        return btn;
    };

    DFrame *contentFrame = new DFrame(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(contentFrame);

    DLabel *label = new DLabel(tr("Select Prompt"), this);
    promptCB = new QComboBox(this);
    promptEdit = new QTextEdit(this);
    promptEdit->setFrameShape(QFrame::NoFrame);
    promptEdit->viewport()->installEventFilter(this);
    promptEdit->installEventFilter(this);
    addBtn = createButton(DStyle::standardIcon(style(), DStyle::SP_IncreaseElement), tr("Add Prompt"));
    delBtn = createButton(DStyle::standardIcon(style(), DStyle::SP_DecreaseElement), tr("Delete Prompt"));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(label);
    topLayout->addWidget(promptCB, 1);

    QVBoxLayout *btnLayout = new QVBoxLayout;
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(delBtn);
    btnLayout->addStretch(1);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(promptEdit);
    layout->addLayout(btnLayout);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(layout);

    addContent(contentFrame);
    addButton(tr("Cancel", "button"));
    addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
}

void SettingDialog::initConnection()
{
    connect(promptCB, &QComboBox::currentTextChanged, this,
            [this] {
                promptEdit->setPlainText(promptCB->currentData().toString());
                bool isDefault = promptCB->currentIndex() == 0;
                delBtn->setEnabled(!isDefault);
                promptEdit->setReadOnly(isDefault);
            });
    connect(addBtn, &QPushButton::clicked, this, &SettingDialog::handleAddPrompt);
    connect(delBtn, &QPushButton::clicked, this, &SettingDialog::handleDeletePrompt);
}

void SettingDialog::updateSettings()
{
    promptCB->addItem("default", defaultIssueFixPrompt());
    const auto &prompts = OptionManager::getInstance()->getValue("Builder", "Prompts").toMap();
    for (auto iter = prompts.cbegin(); iter != prompts.cend(); ++iter) {
        promptCB->addItem(iter.key(), iter.value());
    }

    const auto &curPrompt = OptionManager::getInstance()->getValue("Builder", "CurrentPrompt").toString();
    if (!curPrompt.isEmpty())
        promptCB->setCurrentText(curPrompt);
}

void SettingDialog::handleAddPrompt()
{
    DDialog dlg(this);
    dlg.setIcon(QIcon::fromTheme("ide"));
    dlg.setWindowTitle(tr("Add Prompt"));

    DLineEdit *edit = new DLineEdit(&dlg);
    edit->setPlaceholderText(tr("Please input the name of the prompt"));
    dlg.addContent(edit);
    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
    dlg.getButton(1)->setEnabled(false);
    dlg.setOnButtonClickedClose(false);
    dlg.setFocusProxy(edit);

    connect(edit, &DLineEdit::textChanged, this,
            [edit, &dlg] {
                if (edit->isAlert())
                    edit->setAlert(false);
                dlg.getButton(1)->setEnabled(!edit->text().isEmpty());
            });
    connect(&dlg, &DDialog::buttonClicked, this,
            [this, edit, &dlg](int index) {
                if (index == 1) {
                    const auto &name = edit->text();
                    if (promptCB->findText(name) != -1) {
                        edit->setAlert(true);
                        edit->showAlertMessage(tr("A prompt named \"%1\" already exists").arg(name));
                        edit->lineEdit()->selectAll();
                        edit->setFocus();
                        return;
                    } else {
                        promptCB->addItem(name);
                        promptCB->setCurrentText(name);
                    }
                }
                dlg.close();
            });

    dlg.exec();
}

void SettingDialog::handleDeletePrompt()
{
    DDialog dlg(this);
    dlg.setIcon(QIcon::fromTheme("ide"));
    dlg.setWindowTitle(tr("Delete Prompt"));
    dlg.setMessage(tr("Are you sure you want to delete the \"%1\" prompt").arg(promptCB->currentText()));
    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);

    int ret = dlg.exec();
    if (ret == 1)
        promptCB->removeItem(promptCB->currentIndex());
}

void SettingDialog::handleButtonClicked(int index)
{
    if (index != 1)
        return reject();

    QVariantMap prompts;
    for (int i = 1; i < promptCB->count() - 1; ++i) {   // skip the default prompt
        prompts[promptCB->itemText(i)] = promptCB->itemData(i);
    }

    if (!prompts.isEmpty()) {
        OptionManager::getInstance()->setValue("Builder", "Prompts", prompts);
        OptionManager::getInstance()->setValue("Builder", "CurrentPrompt", promptCB->currentText());
    }

    accept();
}

bool SettingDialog::eventFilter(QObject *obj, QEvent *e)
{
    if (promptEdit && obj == promptEdit->viewport() && e->type() == QEvent::Paint) {
        QPainter painter(promptEdit->viewport());
        painter.setRenderHint(QPainter::Antialiasing);

        auto p = promptEdit->viewport()->palette();
        painter.setPen(Qt::NoPen);
        painter.setBrush(p.brush(QPalette::Active, QPalette::AlternateBase));

        QPainterPath path;
        path.addRoundedRect(promptEdit->viewport()->rect(), 8, 8);
        painter.drawPath(path);
    } else if (promptEdit && obj == promptEdit && e->type() == QEvent::FocusOut && promptCB) {
        promptCB->setItemData(promptCB->currentIndex(), promptEdit->toPlainText());
    }

    return DDialog::eventFilter(obj, e);
}

QString SettingDialog::defaultIssueFixPrompt()
{
    return "如何解决这个问题？如果你提出修复方案，请尽量简洁。对于当前代码，我们遇到以下错误：";
}
