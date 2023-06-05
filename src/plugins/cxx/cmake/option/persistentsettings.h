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
