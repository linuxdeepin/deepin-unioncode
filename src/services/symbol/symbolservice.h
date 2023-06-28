// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
