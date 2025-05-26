// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MACROEXPANDER_H
#define MACROEXPANDER_H

#include <QFileInfo>

#include <functional>

class MacroExpanderPrivate;
class MacroExpander : public QObject
{
    Q_OBJECT
public:
    explicit MacroExpander(QObject *parent = nullptr);
    ~MacroExpander();

    QString value(const QString &variable, bool *found = nullptr);
    QString expand(const QString &unexpandVariable) const;
    QString expandArguments(const QString &args) const;

    using StringFunction = std::function<QString()>;
    using FileFunction = std::function<QFileInfo()>;

    void registerVariable(const QString &variable, const QString &description, const StringFunction &value);
    void registerFileVariables(const QString &prefix, const QString &heading, const FileFunction &base);

    QStringList variables() const;
    QString description(const QString &variable) const;
    QString displayName() const;
    void setDisplayName(const QString &displayName);

private:
    MacroExpanderPrivate *const d;
};

MacroExpander *globalMacroExpander();

#endif   // MACROEXPANDER_H
