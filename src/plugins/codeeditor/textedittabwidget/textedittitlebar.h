// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITTITLEBAR_H
#define TEXTEDITTITLEBAR_H

#include <QStatusBar>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include <iostream>
#include <functional>

class TextEditTitleBarPrivate;
class TextEditTitleBar : public QStatusBar
{
    Q_OBJECT
    TextEditTitleBarPrivate * const d;

public:

    enum StandardButton
    {
        Reload,
        Cancel
    };

    explicit TextEditTitleBar(QWidget *parent = nullptr);
    virtual ~TextEditTitleBar();
    QPushButton * button(StandardButton button);
    static TextEditTitleBar* changedReload(const QString &filePath);

signals:
    void reloadfile();

};

#endif // TEXTEDITTITLEBAR_H
