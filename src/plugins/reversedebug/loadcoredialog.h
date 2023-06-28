// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADCOREDIALOG_H
#define LOADCOREDIALOG_H

#include <QDialog>

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

class LoadCoreDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoadCoreDialog(QWidget *parent = nullptr);
    ~LoadCoreDialog();

    CoredumpRunParameters displayDlg(const QString &traceDir);

signals:

public slots:

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
