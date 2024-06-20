// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLSDIALOG_H
#define BINARYTOOLSDIALOG_H

#include "common/widget/outputpane.h"

#include <DDialog>

class BinaryToolsDialogPrivate;
class BinaryToolsDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit BinaryToolsDialog(QDialog *parent = nullptr);
    ~BinaryToolsDialog();

signals:
    void sigOutputMsg(const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);

public slots:
    void printOutput(const QString &content, OutputPane::OutputFormat format);
    void saveClicked();
    void cancelClicked();
    void useClicked();

private:
    void outputMsg(const QString &content, OutputPane::OutputFormat format);
    bool canRename();
    void doSaveOperation();

    BinaryToolsDialogPrivate *const d = nullptr;
};

#endif // BINARYTOOLSDIALOG_H
