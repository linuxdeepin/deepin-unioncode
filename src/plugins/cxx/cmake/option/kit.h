// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
