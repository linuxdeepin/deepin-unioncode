// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLVIEW_H
#define SYMBOLVIEW_H

#include <DFrame>

class SymbolViewPrivate;
class SymbolView : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit SymbolView(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    bool setSymbolPath(const QString &path);

    using DTK_WIDGET_NAMESPACE::DFrame::show;
    void show(const QPoint &pos);

Q_SIGNALS:
    void hidden();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    SymbolViewPrivate *const d;
};

#endif   // SYMBOLVIEW_H
