// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSLATIONPAGEWIDGET_H
#define TRANSLATIONPAGEWIDGET_H

#include <QWidget>

class TranslationPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TranslationPageWidget(QWidget *parent = nullptr);

private:
   void  initUI();
};

#endif // TRANSLATIONPAGEWIDGET_H
