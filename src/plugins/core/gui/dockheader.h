// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCKHEADER_H
#define DOCKHEADER_H

#include <DWidget>
#include <DToolButton>

class DockHeaderPrivate;
class DockHeader : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit DockHeader(QWidget *parent = nullptr);
    ~DockHeader();

    void addToolButton(DTK_WIDGET_NAMESPACE::DToolButton *btn);

private:
    DockHeaderPrivate *d;
};

#endif // DOCKHEADER_H
