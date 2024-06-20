//// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "advancedsettingsdialog.h"

#include <DLineEdit>
#include <DComboBox>

#include <QFormLayout>

DWIDGET_USE_NAMESPACE

class AdvancedSettingsDialogPrivate : public QObject
{
public:
    explicit AdvancedSettingsDialogPrivate(AdvancedSettingsDialog *qq);

    void initUI();

public:
    AdvancedSettingsDialog *q;

    DLineEdit *hintEdit { nullptr };
    DLineEdit *installEdit { nullptr };
    DComboBox *triggerEventCB { nullptr };
};

AdvancedSettingsDialogPrivate::AdvancedSettingsDialogPrivate(AdvancedSettingsDialog *qq)
    : q(qq)
{
}

void AdvancedSettingsDialogPrivate::initUI()
{
    q->setIcon(QIcon::fromTheme("ide"));
    q->setWindowTitle(AdvancedSettingsDialog::tr("Advanced Settings"));

    QWidget *widget = new QWidget(q);
    QFormLayout *layout = new QFormLayout(widget);

    layout->addRow(AdvancedSettingsDialog::tr("Missing Hint:"), hintEdit = new DLineEdit(q));
    layout->addRow(AdvancedSettingsDialog::tr("Install Command:"), installEdit = new DLineEdit(q));
    layout->addRow(AdvancedSettingsDialog::tr("Trigger Event:"), triggerEventCB = new DComboBox(q));

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
    d->triggerEventCB->setCurrentIndex(d->triggerEventCB->findData(settings.triggerEvent));
}

AdvancedSettings AdvancedSettingsDialog::advancedSettings()
{
    AdvancedSettings st;
    st.missingHint = d->hintEdit->text();
    st.installCommand = d->installEdit->text();
    st.triggerEvent = d->triggerEventCB->currentData().toInt();

    return st;
}
