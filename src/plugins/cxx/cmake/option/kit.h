// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KIT_H
#define KIT_H

#include "global_define.h"

class KitPrivate;
class Kit : public QObject
{
    Q_OBJECT
public:
    explicit Kit(QObject *parent = nullptr);
    Kit(const Kit &other);
    ~Kit();

    QString id() const;
    void setId(const QString &id);
    QString kitName() const;
    void setKitName(const QString &name);
    Option ccompiler() const;
    void setCCompiler(const Option &opt);
    Option cxxcompiler() const;
    void setCXXCompiler(const Option &opt);
    Option debugger() const;
    void setDebugger(const Option &opt);
    Option cmakeTool() const;
    void setCMakeTool(const Option &opt);
    QString cmakeGenerator() const;
    void setCMakeGenerator(const QString &cg);

    bool isValid() const;
    void copyFrom(const Kit &other);
    QVariantMap toVariantMap();
    static QVariantMap toVariantMap(const Kit &kit);
    static Kit fromVariantMap(const QVariantMap &map);

    Kit &operator=(const Kit &other);
    bool operator==(const Kit &other) const;

private:
    KitPrivate *const d;
};

#endif   // KIT_H
