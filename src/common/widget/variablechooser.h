// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VARIABLECHOOSER_H
#define VARIABLECHOOSER_H

#include "common/util/macroexpander.h"

#include <DDialog>

#include <functional>

using MacroExpanderProvider = std::function<MacroExpander *()>;

class QLineEdit;
class VariableChooserPrivate;
class VariableChooser : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit VariableChooser(QWidget *parent = nullptr);
    ~VariableChooser();

    void addMacroExpander(MacroExpander *expander);
    void addSupportedEdit(QLineEdit *edit, const QByteArray &ownName = QByteArray());

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    VariableChooserPrivate *const d;
};

#endif   // VARIABLECHOOSER_H
