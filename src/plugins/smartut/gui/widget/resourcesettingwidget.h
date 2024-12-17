// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESOURCESETTINGWIDGET_H
#define RESOURCESETTINGWIDGET_H

#include "common/itemnode.h"
#include "gui/projecttreeview.h"

#include <DFrame>
#include <DComboBox>
#include <DLineEdit>
#include <DSuggestButton>

class ResourceSettingWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit ResourceSettingWidget(QWidget *parent = nullptr);

    bool apply();
    void updateSettings();
    QStringList selectedFileList() const;
    QString selectedProject() const;
    QString targetLocation() const;

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void initUI();
    void initConnection();
    QStringList selectedFileList(NodeItem *item) const;

private Q_SLOTS:
    void handleProjectChanged();
    void handleSelectLocation();

private:
    DTK_WIDGET_NAMESPACE::DComboBox *projectCB { nullptr };
    DTK_WIDGET_NAMESPACE::DLineEdit *targetLocationEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DSuggestButton *targetSelBtn { nullptr };
    ProjectTreeView *prjView { nullptr };
    dpfservice::ProjectInfo prjInfoCache;
};

#endif   // RESOURCESETTINGWIDGET_H
