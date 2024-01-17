// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITORMENU_H
#define TEXTEDITORMENU_H

#include "base/abstracteditormenu.h"

#include <QMap>

class TextEditor;
class TextEditorMenu : public AbstractEditorMenu
{
    Q_OBJECT
public:
    explicit TextEditorMenu(QObject *parent = nullptr);

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual AbstractEditorMenu *menu(QAction *action) const override;

private:
    QMap<QString, QAction *> predicateAction;   // id -- instance
    QMap<QString, QString> predicateName;   // id -- text
    TextEditor *editor { nullptr };
};

#endif   // TEXTEDITORMENU_H
