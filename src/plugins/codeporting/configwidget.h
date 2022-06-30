/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QDialog>

class ConfigWidgetPrivate;
class ConfigWidget : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = nullptr);

signals:
    void sigStartPorting();

public slots:
    void configDone();

private:
    void showEvent(QShowEvent *) override;
    void initializeUi();
    void setDefaultValue();
    void refreshUi();
    void setupUi(QWidget *);
    void resetUi();
    void refreshDetail();
    bool saveCfg();
    bool restore();
    QString configFilePath();

    ConfigWidgetPrivate *const d;
};

#endif // CONFIGWIDGET_H
