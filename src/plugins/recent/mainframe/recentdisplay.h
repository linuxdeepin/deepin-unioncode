// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTDISPLAY_H
#define RECENTDISPLAY_H

#include <DWidget>
#include "common/common.h"

class RecentDisplayPrivate;
class RecentDisplay : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
    RecentDisplayPrivate *const d;
public:
    enum ItemRole{
        ProjectKitName = Qt::ItemDataRole::UserRole,
        ProjectLanguage,
        ProjectWorkspace,
    };
    explicit RecentDisplay(DTK_WIDGET_NAMESPACE::DWidget *parent = nullptr);
    virtual ~RecentDisplay() override;
    static RecentDisplay *instance();

public slots:
    void addDocument(const QString &filePath);
    void addProject(const QString &kitName,
                    const QString &language,
                    const QString &workspace);

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
    bool isProAndDocNull();

    void showEvent(QShowEvent *event) override;
};

#endif // RECENTDISPLAY_H
