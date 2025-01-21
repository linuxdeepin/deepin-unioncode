// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMARTUTWIDGET_H
#define SMARTUTWIDGET_H

#include "common/itemnode.h"

#include <DToolButton>
#include <DComboBox>

#include <QStackedWidget>

namespace dpfservice {
class WindowService;
}

class ProjectTreeView;
class SettingDialog;
class NodeItem;
class SmartUTWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SmartUTWidget(QWidget *parent = nullptr);

    void showSettingDialog();

public Q_SLOTS:
    void generateAllUTFiles();
    void generateUTFiles(NodeItem *item);
    void continueToGenerateAll();
    void continueToGenerate(NodeItem *item);
    void updateModelList();
    void updateItemState(NodeItem *item);

private:
    void initUI();
    void initConnection();
    QWidget *createBlankPage();
    QWidget *createMainWidget();

    void setProjectList(const QString &workspace, const QStringList &fileList);
    std::vector<std::unique_ptr<FileNode>> createFileNodes(const QString &workspace, const QStringList &fileList);
    bool checkModelValid();

private:
    QStackedWidget *mainWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DComboBox *modelCB { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *generateBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *continueBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *stopBtn { nullptr };
    ProjectTreeView *prjView { nullptr };
    SettingDialog *settingDlg { nullptr };
    dpfservice::WindowService *winSrv { nullptr };
};

#endif   // SMARTUTWIDGET_H
