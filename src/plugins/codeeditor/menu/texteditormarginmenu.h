// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITORMARGINMENU_H
#define TEXTEDITORMARGINMENU_H

#include "base/abstracteditormenu.h"

class TextEditorMarginMenu : public AbstractEditorMenu
{
    Q_OBJECT
public:
    explicit TextEditorMarginMenu(QObject *parent = nullptr);

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual AbstractEditorMenu *menu(QAction *action) const override;
};

#endif // TEXTEDITORMARGINMENU_H
