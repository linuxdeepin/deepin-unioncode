// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLSDIALOG_H
#define BINARYTOOLSDIALOG_H

#include <DDialog>

class BinaryToolsDialogPrivate;
class BinaryToolsDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit BinaryToolsDialog(QDialog *parent = nullptr);
    ~BinaryToolsDialog();

public slots:
    void saveClicked();

private:
    BinaryToolsDialogPrivate *const d = nullptr;
};

#endif // BINARYTOOLSDIALOG_H
