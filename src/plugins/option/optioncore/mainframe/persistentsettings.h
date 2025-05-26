// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERSISTENTSETTINGS_H
#define PERSISTENTSETTINGS_H

#include <QVariant>
#include <QString>

class PersistentSettingsReader
{
public:
    PersistentSettingsReader();
    QVariant restoreValue(const QString &variable, const QVariant &defaultValue = QVariant()) const;
    QVariantMap restoreValues() const;
    bool load(const QString &QString);

private:
    QMap<QString, QVariant> valueMap;
};

class PersistentSettingsWriter
{
public:
    PersistentSettingsWriter(const QString &_fileName, const QString &_docType);
    ~PersistentSettingsWriter();

    bool save(const QVariantMap &data, QString *errorString) const;
#ifdef QT_GUI_LIB
    bool save(const QVariantMap &data, QWidget *parent) const;
#endif

    QString getFileName() const;

    void setContents(const QVariantMap &data);

private:
    bool write(const QVariantMap &data, QString *errorString) const;

    const QString fileName;
    const QString docType;
    mutable QMap<QString, QVariant> savedData;
};

#endif // PERSISTENTSETTINGS_H
