// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSLATIONPAGEWIDGET_H
#define TRANSLATIONPAGEWIDGET_H

#include <DWidget>
#include <DComboBox>
#include <DSuggestButton>
#include <DSpinner>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
class QComboBox;
QT_END_NAMESPACE

class CodeEditComponent;
class TranslationPageWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TranslationPageWidget(QWidget *parent = nullptr);
    void setInputEditText(const QString &text);
    void cleanOutputEdit();

public Q_SLOTS:
    void onTranslateBtnClicked();
    void onRecevieTransCode(const QString &code);

private:
    void initUI();
    void initConnection();

    DSpinner *spinner { nullptr };
    DSuggestButton *transBtn { nullptr };
    DComboBox *langComboBox { nullptr };
    CodeEditComponent *inputEdit { nullptr };
    CodeEditComponent *outputEdit { nullptr };
};

#endif   // TRANSLATIONPAGEWIDGET_H
