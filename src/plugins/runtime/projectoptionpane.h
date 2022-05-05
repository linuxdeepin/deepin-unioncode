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
#ifndef PROJECTOPTIONPANE_H
#define PROJECTOPTIONPANE_H

#include <QGroupBox>

class QPushButton;
class KitsManagerWidget;
namespace dpfservice {
    class ProjectService;
}
class ProjectOptionPane : public QGroupBox
{
    Q_OBJECT
public:
    explicit ProjectOptionPane(QWidget *parent = nullptr);
    ~ProjectOptionPane();

signals:
    void activeBuildCfgPane();
    void activeRunCfgPane();

public slots:
    void buildBtnClicked();
    void runBtnClicked();
    void showKitDialog();

private:
    void setupUI();
    void initializeKitManageWidget();

    QPushButton *buildButton = nullptr;
    QPushButton *runButton = nullptr;
    KitsManagerWidget *kitManagerWidget = nullptr;

    dpfservice::ProjectService *projectService = nullptr;
};

#endif // PROJECTOPTIONPANE_H
