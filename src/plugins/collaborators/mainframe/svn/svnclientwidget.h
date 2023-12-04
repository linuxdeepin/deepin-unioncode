// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SVNCLIENTWIDGET_H
#define SVNCLIENTWIDGET_H

#include <QMainWindow>
#include <QSet>

class ReposWidget;
class QPinnableTabWidget;
class SvnClientWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit SvnClientWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    bool isSvnDir(const QString &repoPath);

public slots:
    void addRepoTab(const QString &repoPath,
                    const QString &user = "", const QString &passwd = "");
    void addNewRepoTab(const QString &repoPathArg,
                       const QString &user = "", const QString &passwd = "");
    void showCheckoutDialog();
    void showOpenLocalRepos();

private slots:
    void doCheckoutRepos(const QString &remote, const QString &local,
                         const QString &user, const QString &passwd);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QSet<QString> mCurrentRepos;
    QPinnableTabWidget *mRepos{nullptr};
};

#endif // SVNCLIENTWIDGET_H
