// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONLISTVIEW_H
#define SESSIONLISTVIEW_H

#include "sessionmodel.h"

#include <QTreeView>

class SessionNameInputDialog;
class SessionListView : public QTreeView
{
    Q_OBJECT
public:
    explicit SessionListView(QWidget *parent = nullptr);

    QString currentSession() const;
    void selectSession(const QString &session);
    void selectActiveSession();

public Q_SLOTS:
    void createSession();
    void removeSelectedSessions();
    void cloneCurrentSession();
    void renameCurrentSession();
    void switchToCurrentSession();

Q_SIGNALS:
    void sessionsSelected(const QStringList &sessions);
    void sessionCreated(const QString &session);

private:
    void initUI();
    void initConnections();

    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    QStringList selectedSessions() const;
    void runInputDialog(SessionNameInputDialog *dialog,
                        std::function<void(const QString &)> handler);

private:
    SessionModel model;
};

#endif   // SESSIONLISTVIEW_H
