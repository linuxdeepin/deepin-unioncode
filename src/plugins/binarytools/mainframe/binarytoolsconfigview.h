/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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

#ifndef BINARYTOOLSCONFIGVIEW_H
#define BINARYTOOLSCONFIGVIEW_H

#include <QWidget>

class BinaryToolsConfigViewPrivate;
class BinaryToolsConfigView : public QWidget
{
    Q_OBJECT
public:
    explicit BinaryToolsConfigView(QWidget *parent = nullptr);
    ~BinaryToolsConfigView();

    bool saveConfig();
    void readConfig();

    QList<QString> getProgramList();
    QList<QStringList> getArgumentsList();
    QList<QString> getWorkingDirList();
    QList<QMap<QString, QVariant>> getEnvironmentList();

signals:
    void comboChanged();
    void useCombinationCommand();

private:
    void updateView(const QString &command);
    void currentConfigChanged(const QString &text);
    void initializeCombo();
    void initializeCombinationDialog();
    void addCompatConfig();
    void deleteCompatConfig();
    void renameCompatConfig();
    void combineCompatConfig();
    void setConfigWidget();
    void findWorkPath();
    void appendCommand(const QString &name);
    QString uniqueName(const QString &name);

    BinaryToolsConfigViewPrivate *const d = nullptr;
};

#endif // BINARYTOOLSCONFIGVIEW_H
