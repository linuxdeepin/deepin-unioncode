// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLWIDGET_H
#define SYMBOLWIDGET_H

#include <QWidget>

class TextEditor;
class SymbolWidget;
class SymbolWidgetPrivate;
class SymbolWidgetGenerator : public QObject
{
    Q_OBJECT
public:
    static SymbolWidgetGenerator *instance();

    void registerToDock(QWidget *parent);
    SymbolWidget *symbolWidget();

private:
    SymbolWidgetGenerator() = default;

    SymbolWidget *symbWidget { nullptr };
};

class SymbolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SymbolWidget(QWidget *parent = nullptr);
    ~SymbolWidget();

    void setEditor(TextEditor *editor);
    void registerDockHeader();

protected:
    void showEvent(QShowEvent *event) override;

private:
    SymbolWidgetPrivate *const d;
};

#endif   // SYMBOLWIDGET_H
