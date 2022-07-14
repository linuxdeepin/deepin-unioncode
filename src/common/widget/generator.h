/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef GENERATOR_H
#define GENERATOR_H

#include <QObject>
#include <QVariant>

class Generator : public QObject
{
    Q_OBJECT
public:
    struct Procedure
    {
        const QString &message;
        const int current;
        int max = 100;
    };
    explicit Generator(QObject *parent = nullptr);
    QString errorString();

signals:
    void message(const Procedure &procedure);
    void started();
    void finished(bool isNormal = true);

protected:
    bool setErrorString(const QString &error); //子類調用
    bool setProperty(const QString &name, const QVariant &value); //子類調用
    QVariant property(const QString &name) const; //子類調用
};

#endif // GENERATOR_H
