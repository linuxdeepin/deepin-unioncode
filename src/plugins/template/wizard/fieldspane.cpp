// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fieldspane.h"

#include <QFormLayout>
#include <QJsonArray>

#include <DLineEdit>
#include <DComboBox>
#include <DLabel>
#include <DSuggestButton>
#include <DFileDialog>

DWIDGET_USE_NAMESPACE
FieldsPane::FieldsPane(const templateMgr::Page &pageInfo, DWidget *parent)
    : AbstractPane(parent), page(pageInfo)
{
    setupUi();
}

void FieldsPane::setupUi()
{
    DLabel *title = new DLabel(this);
    title->setText(page.shortTitle);
    
    QFormLayout *mainLayout = new QFormLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    mainLayout->addRow(title);
    
    for (auto item : page.items) {
        if (item.type == "ComboBox") {
            DLabel *lb = new DLabel(item.displayName, this);
            DComboBox *cb = new DComboBox(this);
            QJsonArray cbItems = item.data.value("items").toArray();
            connect(cb, &DComboBox::currentTextChanged, this, [=](const QString &text){
                value[item.key] = text;
            });
            
            for (auto cbItem : cbItems) {
                if (cbItem.isObject())
                    cb->addItem(cbItem.toObject().value("value").toString());
                else
                    cb->addItem(cbItem.toString());
            }
            mainLayout->addRow(lb, cb);
        } else if (item.type == "LineEdit") {
            DLabel *lb = new DLabel(item.displayName, this);
            DLineEdit *le = new DLineEdit(this);
            //todo(zta : process data {trText, trDisabledText}
            mainLayout->addRow(lb, le);
            
            connect(le, &DLineEdit::editingFinished, this, [=](){
                value[item.key] = le->text();
            });
        } //todo (zta : spacer,checkbox 
    }
}

QMap<QString, QVariant> FieldsPane::getValue()
{
    return value;
}
