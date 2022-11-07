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
#ifndef SYMBOLSERVICE_H
#define SYMBOLSERVICE_H

#include <framework/framework.h>

#include <QTabWidget>

namespace dpfservice {

class SymbolService final : public dpf::PluginService,
        dpf::AutoServiceRegister<SymbolService>
{
    Q_OBJECT
    Q_DISABLE_COPY(SymbolService)
public:
    static QString name()
    {
        return "org.deepin.service.SymbolService";
    }

    explicit SymbolService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }
};

/* MainWindow codeediter workspace title,
 * use in window service swtich workspace
 */
inline const QString MWCWT_SYMBOL {QTabWidget::tr("Symbol")};

} //namespace dpfservice

#endif // SYMBOLSERVICE_H
