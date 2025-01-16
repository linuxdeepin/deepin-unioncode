// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMARTUTWIDGET_H
#define SMARTUTWIDGET_H

#include <DToolButton>
#include <DComboBox>

#include <QStackedWidget>

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
    void createUTFiles();
    void createUTFiles(NodeItem *item);
    void updateModelList();
    void updateItemState(NodeItem *item);

private:
    void initUI();
    void initConnection();
    QWidget *createBlankPage();
    QWidget *createMainWidget();

    void fillProjectView(const QString &workspace, const QStringList &fileList);

private:
    QStackedWidget *mainWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DComboBox *modelCB { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *generateBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *stopBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *runBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *reportBtn { nullptr };
    ProjectTreeView *prjView { nullptr };
    SettingDialog *settingDlg { nullptr };
};

#endif   // SMARTUTWIDGET_H
