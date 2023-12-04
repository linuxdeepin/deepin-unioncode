// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPOSWIDGET_H
#define REPOSWIDGET_H

#include "basetype.h"

class AmendsWidget;
class CommitHistoryWidget;
class ReposWidgetPrivate;
class ReposWidget : public DWidget
{
    Q_OBJECT
    ReposWidgetPrivate *const d;
public:
    explicit ReposWidget(QWidget* parent = nullptr);
    virtual ~ReposWidget();

    QString getReposPath() const;
    void setReposPath(const QString &path);
    QString getName() const;
    void setName(const QString &value);
    QString getPasswd() const;
    void setPasswd(const QString &value);
    void setLogginDisplay(const QString &name = "", const QString &passwd = "");

public slots:
    void doLoggin(); // main func
    void doUpdateRepos();
    void doRefresh();
    void doAmendsCommit();
    void doAmendsRevertAll();
    void doDiffFileAtRevision();

private:
    bool add(const QString &display);
    bool revert(const QString &display);

private slots:
    void loadRevisionFiles();
    void reloadRevisionFiles();
    void loadHistory();
    void reloadHistory();
    void modFileMenu(const RevisionFile &file, const QPoint &pos);
    void historyDataClicked(const QModelIndex &index);
    void historyFileClicked(const QModelIndex &index);
    void setSrcViewReviFiles(const QString &path);

private:
    void fileModifyMenu();
    static bool testUserLoggin(const QString &reposPath,
                               const QString &name,
                               const QString &passwd);
    QWidget *initControlBar();
};

#endif // REPOSWIDGET_H
