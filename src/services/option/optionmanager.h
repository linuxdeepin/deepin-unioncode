// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONMANAGER_H
#define OPTIONMANAGER_H

#include "optiondatastruct.h"
#include <QObject>
#include <QVariant>

class OptionManagerPrivate;
class OptionManager : public QObject
{
    Q_OBJECT
public:
    static OptionManager *getInstance();

    void updateData();

    QString getMavenToolPath();
    QString getJdkToolPath();
    QString getGradleToolPath();
    QString getPythonToolPath();
    QString getCMakeToolPath();
    QString getNinjaToolPath();
    QString getCxxDebuggerToolPath();
    QString getJSToolPath();
    QString getToolPath(const QString &kit);
    QVariant getValue(const QString &category, const QStringList &properties);
signals:

private:
    explicit OptionManager(QObject *parent = nullptr);
    virtual ~OptionManager() override;

    OptionManagerPrivate *const d;
};

#endif // OPTIONMANAGER_H
