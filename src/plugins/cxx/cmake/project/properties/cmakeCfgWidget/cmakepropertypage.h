// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEPROPERTYPAGE_H
#define CMAKEPROPERTYPAGE_H

#include "cmakeitemmodel.h"

#include <DWidget>

#include <QString>

class CMakePropertyPagePrivate;
class CMakePropertyPage : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit CMakePropertyPage(QWidget *parent = nullptr);
    ~CMakePropertyPage();

    void saveConfigToCacheFile(const QString &cacheFile);
    void getItemsFromCacheFile(const QString &cacheFile);

    void setupUi();

signals:
    void cacheFileUpdated();

private:
    CMakePropertyPagePrivate *d;
};

#endif // CMAKEPROPERTYPAGE_H
