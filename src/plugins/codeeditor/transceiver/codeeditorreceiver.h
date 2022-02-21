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
#ifndef CODEEDITORRECEIVER_H
#define CODEEDITORRECEIVER_H

#include <framework/framework.h>

class CodeEditorReceiver: public dpf::EventHandler, dpf::AutoEventHandlerRegister<CodeEditorReceiver>
{
    friend class dpf::AutoEventHandlerRegister<CodeEditorReceiver>;
public:
    explicit CodeEditorReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};


class DpfEventMiddleware : public QObject
{
    Q_OBJECT
    DpfEventMiddleware(){}
    DpfEventMiddleware(const DpfEventMiddleware&) = delete;

public:
    static DpfEventMiddleware* instance();

signals:
    void toOpenFile(const QString &filePath, const QString &rootPath);
    void toRunFileLine(const QString &filePath, int line);
};

#endif // CODEEDITORRECEIVER_H
