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
#ifndef KIT_H
#define KIT_H

#include <QHash>
#include <QVariant>

#include <memory>

class KitPrivate;
class Kit : public QObject
{
    Q_OBJECT
public:
    explicit Kit(QString id = "", QObject *parent = nullptr);
    explicit Kit(const QVariantMap &data);
    ~Kit();

    QString displayName() const;
    void setUnexpandedDisplayName(const QString &name);
    QString id() const;
    QList<QString> allKeys() const;
    QVariant value(QString key, const QVariant &unset = QVariant()) const;
    bool hasValue(QString key) const;
    void setValue(QString key, const QVariant &value);

    void setDefaultOutput(QString &defaultOutput);
    const QString &getDefaultOutput() const;

    void copyFrom(const Kit &k);

    Kit(const Kit &other);
    Kit &operator=(const Kit &other);
signals:

public slots:

private:
    QVariantMap toMap() const;

    const std::unique_ptr<KitPrivate> d;
};

#endif // KIT_H
