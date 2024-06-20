// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLSCONFIGVIEW_H
#define BINARYTOOLSCONFIGVIEW_H

#include <DWidget>

class BinaryToolsConfigViewPrivate;
class BinaryToolsConfigView : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    struct RenameInfo{
        QString oldName = "";
        QString name = "";
        int index = -1;
        QString uniName = "";
    };

    explicit BinaryToolsConfigView(QWidget *parent = nullptr);
    ~BinaryToolsConfigView();

    bool saveConfig();
    void readConfig();
    void renameConfigOperation(const QString &name, const QString &oldname, const QString &uniName);
    void resetConfigOperation(const QString &oldname, int index);

    bool hasRename = false; // Whether the rename button was clicked
    RenameInfo renameInfo;

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
