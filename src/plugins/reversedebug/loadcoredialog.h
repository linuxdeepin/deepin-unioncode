// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADCOREDIALOG_H
#define LOADCOREDIALOG_H

#include <DDialog>

namespace ReverseDebugger {
namespace Internal {

class StartCoredumpDialogPrivate;
class CoredumpRunParameters
{
public:
    int pid = 0;

    // trace directory.
    QString tracedir;
};

class LoadCoreDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit LoadCoreDialog(QWidget *parent = nullptr);
    ~LoadCoreDialog();

    CoredumpRunParameters displayDlg(const QString &traceDir);

signals:

public Q_SLOTS:
    void onButtonClicked(const int &index);

private:
    void setupUi();
    void updatePid();
    void historyIndexChanged(int);
    void showFileDialog();

    StartCoredumpDialogPrivate *const d;
};

}   // namespace ReverseDebugger
}   // namespace Internal

#endif   // LOADCOREDIALOG_H
