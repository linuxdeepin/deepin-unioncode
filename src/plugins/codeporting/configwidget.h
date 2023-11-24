// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <DDialog>
#include <DWidget>

class ConfigWidgetPrivate;
class ConfigWidget : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = nullptr);

signals:
    void sigStartPorting(const QString &project, const QString &srcCPU, const QString &destCPU);

public slots:
    void configDone();

private:
    void showEvent(QShowEvent *) override;
    void initializeUi();
    void setDefaultValue();
    void refreshUi();
    void resetUi();
    void refreshDetail();
    bool saveCfg();
    bool restore();
    QString configFilePath();

    ConfigWidgetPrivate *const d;
};

#endif // CONFIGWIDGET_H
