//// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "advancedsettingsdialog.h"

#include "common/widget/variablechooser.h"

#include <DLineEdit>
#include <DComboBox>
#include <DIconTheme>

#include <QFormLayout>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class AdvancedSettingsDialogPrivate : public QObject
{
public:
    explicit AdvancedSettingsDialogPrivate(AdvancedSettingsDialog *qq);

    void initUI();

public:
    AdvancedSettingsDialog *q;

    DLineEdit *hintEdit { nullptr };
    DLineEdit *installEdit { nullptr };
    DLineEdit *channelEdit { nullptr };
    DComboBox *triggerEventCB { nullptr };
};

AdvancedSettingsDialogPrivate::AdvancedSettingsDialogPrivate(AdvancedSettingsDialog *qq)
    : q(qq)
{
}

void AdvancedSettingsDialogPrivate::initUI()
{
    q->setIcon(DIconTheme::findQIcon("ide"));
    q->setWindowTitle(AdvancedSettingsDialog::tr("Advanced Settings"));

    QWidget *widget = new QWidget(q);
    QFormLayout *layout = new QFormLayout(widget);

    layout->addRow(AdvancedSettingsDialog::tr("Missing Hint:"), hintEdit = new DLineEdit(q));
    layout->addRow(AdvancedSettingsDialog::tr("Install Command:"), installEdit = new DLineEdit(q));
    layout->addRow(AdvancedSettingsDialog::tr("Channel Data:"), channelEdit = new DLineEdit(q));
    layout->addRow(AdvancedSettingsDialog::tr("Trigger Event:"), triggerEventCB = new DComboBox(q));

    VariableChooser *chooser = new VariableChooser(q);
    chooser->addSupportedEdit(channelEdit->lineEdit());

    triggerEventCB->addItem(AdvancedSettingsDialog::tr("None"), NoEvent);
    triggerEventCB->addItem(AdvancedSettingsDialog::tr("Document Saved"), DocumentSaveEvent);

    q->addContent(widget);
    q->addButton(AdvancedSettingsDialog::tr("Cancel", "button"), false, DDialog::ButtonNormal);
    q->addButton(AdvancedSettingsDialog::tr("OK", "button"), true, DDialog::ButtonRecommend);
}

AdvancedSettingsDialog::AdvancedSettingsDialog(QWidget *parent)
    : DDialog(parent),
      d(new AdvancedSettingsDialogPrivate(this))
{
    d->initUI();
}

void AdvancedSettingsDialog::setAdvancedSettings(const AdvancedSettings &settings)
{
    d->hintEdit->setText(settings.missingHint);
    d->installEdit->setText(settings.installCommand);
    d->channelEdit->setText(settings.channelData);
    d->triggerEventCB->setCurrentIndex(d->triggerEventCB->findData(settings.triggerEvent));
}

AdvancedSettings AdvancedSettingsDialog::advancedSettings()
{
    AdvancedSettings st;
    st.missingHint = d->hintEdit->text();
    st.installCommand = d->installEdit->text();
    st.channelData = d->channelEdit->text();
    st.triggerEvent = d->triggerEventCB->currentData().toInt();

    return st;
}
