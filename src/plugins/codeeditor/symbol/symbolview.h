// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLVIEW_H
#define SYMBOLVIEW_H

#include "lsp/protocol/new/languagefeatures.h"

#include <DFrame>

class SymbolViewPrivate;
class SymbolView : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    enum ClickMode {
        Click = 0,
        DoubleClick
    };

    explicit SymbolView(ClickMode mode, QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    bool setSymbolPath(const QString &path);
    void select(const QString &text);
    void selectSymbol(const QString &symbol, int line, int col);
    void expandAll();
    void collapseAll();
    bool isEmpty();
    void setClickToHide(bool enable);

    using DTK_WIDGET_NAMESPACE::DFrame::show;
    void show(const QPoint &pos);

Q_SIGNALS:
    void hidden();

protected:
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    SymbolViewPrivate *const d;
};

Q_DECLARE_METATYPE(newlsp::Range)

#endif   // SYMBOLVIEW_H
