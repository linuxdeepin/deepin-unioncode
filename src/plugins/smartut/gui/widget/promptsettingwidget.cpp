// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "promptsettingwidget.h"
#include "manager/smartutmanager.h"

#include <DLabel>
#include <DLineEdit>
#include <DDialog>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

PromptSettingWidget::PromptSettingWidget(QWidget *parent)
    : DFrame(parent)
{
    initUI();
    initConnection();
}

void PromptSettingWidget::initUI()
{
    auto createButton = [this](const QIcon &icon, const QString &tips) {
        auto btn = new DPushButton(this);
        btn->setIconSize({ 16, 16 });
        btn->setIcon(icon);
        btn->setToolTip(tips);
        return btn;
    };

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    DLabel *label = new DLabel(tr("Select Prompt"), this);
    promptCB = new DComboBox(this);
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

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(promptEdit);
    contentLayout->addLayout(btnLayout);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(contentLayout);
}

void PromptSettingWidget::initConnection()
{
    connect(promptCB, &DComboBox::currentTextChanged, this,
            [this] {
                promptEdit->setPlainText(promptCB->currentData().toString());
                const auto &defaultPrompts = SmartUTManager::instance()->utSetting()->defaultValue(kGeneralGroup, kPrompts).toMap();
                bool isDefault = defaultPrompts.contains(promptCB->currentText());
                delBtn->setEnabled(!isDefault);
                promptEdit->setReadOnly(isDefault);
            });
    connect(addBtn, &DPushButton::clicked, this, &PromptSettingWidget::handleAddPrompt);
    connect(delBtn, &DPushButton::clicked, this, &PromptSettingWidget::handleDeletePrompt);
}

void PromptSettingWidget::updateSettings()
{
    promptCB->clear();

    auto setting = SmartUTManager::instance()->utSetting();
    const auto &prompts = setting->value(kGeneralGroup, kPrompts).toMap();
    for (auto iter = prompts.cbegin(); iter != prompts.cend(); ++iter) {
        promptCB->addItem(iter.key(), iter.value());
    }

    const auto &activePrompt = setting->value(kActiveGroup, kActivePrompt).toString();
    promptEdit->setPlainText(prompts.value(activePrompt).toString());
}

void PromptSettingWidget::handleAddPrompt()
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

void PromptSettingWidget::handleDeletePrompt()
{
    DDialog dlg(this);
    dlg.setIcon(QIcon::fromTheme("ide"));
    dlg.setWindowTitle(tr("Delete Prompt"));
    dlg.setMessage(tr("Are you sure you want to delete the \"%1\" prompt").arg(promptCB->currentText()));
    dlg.addButton(tr("Cancel", "button"));
    dlg.addButton(tr("Ok", "button"), true, DDialog::ButtonRecommend);

    int ret = dlg.exec();
    if (ret == 1)
        promptCB->removeItem(promptCB->currentIndex());
}

void PromptSettingWidget::apply()
{
    QVariantMap prompts;
    for (int i = 0; i < promptCB->count(); ++i) {
        prompts.insert(promptCB->itemText(i), promptCB->itemData(i));
    }

    auto setting = SmartUTManager::instance()->utSetting();
    setting->setValue(kGeneralGroup, kPrompts, prompts);
    setting->setValue(kActiveGroup, kActivePrompt, promptCB->currentText());
}

bool PromptSettingWidget::eventFilter(QObject *obj, QEvent *e)
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

    return DFrame::eventFilter(obj, e);
}
