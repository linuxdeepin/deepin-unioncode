// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTDISPLAYWIDGET_H
#define RECENTDISPLAYWIDGET_H

#include <DWidget>
#include "common/common.h"

class RecentDisplayWidgetPrivate;
class RecentDisplayWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
    RecentDisplayWidgetPrivate *const d;

public:
    explicit RecentDisplayWidget(DTK_WIDGET_NAMESPACE::DWidget *parent = nullptr);
    virtual ~RecentDisplayWidget() override;
    static RecentDisplayWidget *instance();

public slots:
    void addDocument(const QString &filePath);
    void addProject(const QString &kitName,
                    const QString &language,
                    const QString &workspace);
    void addSession(const QString &session);
    void removeSession(const QString &session);
    void updateSessions();

private slots:
    void doDoubleClickedProject(const QModelIndex &index);
    void doDoubleCliekedDocument(const QModelIndex &index);

    void btnOpenFileClicked();
    void btnOpenProjectClicked();
    void btnNewFileOrProClicked();

    void clearDocList();
    void clearProList();

private:
    void initializeUi();
    void initConnect();
    void initData();
    bool isProAndDocNull();
    QVariantMap parseProjectInfo(const QJsonObject &obj);

    void showEvent(QShowEvent *event) override;
};

#endif   // RECENTDISPLAYWIDGET_H
