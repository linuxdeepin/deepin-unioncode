// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTEDITSPLITTER_H
#define TEXTEDITSPLITTER_H

#include "textedittabwidget.h"

#include <QObject>
#include <QGridLayout>
#include <QSplitter>
#include <QList>
#include <QPair>

class TextEditSplitter : public QWidget
{
    Q_OBJECT
public:
    friend class TextEditTabWidget;

    explicit TextEditSplitter(QWidget *parent = nullptr);
    QSplitter *getSplitter() const;

    QString getSelectedText();
    QString getCursorBeforeText();
    QString getCursorAfterText();
    void replaceSelectedText(const QString &text);
    void showTips(const QString &tips);
    void insertText(const QString &text);
    void undo();

public slots:
    void doSplit(Qt::Orientation orientation, const newlsp::ProjectKey &key, const QString &file);
    void doClose();
    void doSelected(bool state);
    void doShowSplit();
    static TextEditSplitter *instance();

private:
    virtual ~TextEditSplitter() override;
    void updateClose(QSplitter *splitter, TextEditTabWidget *textEditTabWidget);
    QVBoxLayout *vLayout = nullptr;
    QSplitter *mainSplitter = nullptr;
    TextEditTabWidget *tabWidget = nullptr;
    QHash<TextEditTabWidget *, bool> tabWidgets;
    QHash<QSplitter *, QPair<TextEditTabWidget *, TextEditTabWidget*>> splitters;
};

#endif // TEXTEDITSPLITTER_H
