// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTINFODIALOG_H
#define PROJECTINFODIALOG_H

#include <QTextBrowser>
#include <QDialog>
#include <QVBoxLayout>

class ProjectInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectInfoDialog(QWidget *parent = nullptr,
                               Qt::WindowFlags f = Qt::WindowFlags());
    void setPropertyText(const QString &text);
private:
    QVBoxLayout *vLayout {nullptr};
    QTextBrowser *textBrowser {nullptr};
};

#endif // PROJECTINFODIALOG_H
