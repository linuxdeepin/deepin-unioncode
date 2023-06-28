// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KITMANAGER_H
#define KITMANAGER_H

#include "kit.h"

#include <QObject>

class KitManager : public QObject
{
    Q_OBJECT
public:
    static KitManager *instance();
    ~KitManager() override;

    void save();

    void setSelectedKit(Kit &kit);
    const Kit &getSelectedKit();

    QString getDefaultOutputPath() const;

signals:

public slots:
private:
    explicit KitManager(QObject *parent = nullptr);

    void restoreKits();

    class KitList
    {
    public:
        KitList() {}
        QString defaultKit;
        std::vector<std::unique_ptr<Kit>> kits;
    };

    KitList restoreKits(const QString &fileName);
};

#endif // KITMANAGER_H
