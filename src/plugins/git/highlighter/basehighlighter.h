// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEHIGHLIGHTER_H
#define BASEHIGHLIGHTER_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QTextDocument;
class QTextCharFormat;
QT_END_NAMESPACE

class BaseHighlighterPrivate;
class BaseHighlighter : public QObject
{
    Q_OBJECT
public:
    explicit BaseHighlighter(QObject *parent = nullptr);
    explicit BaseHighlighter(QTextDocument *doc);

    void setDocument(QTextDocument *doc);
    QTextDocument *document() const;

    static QList<QColor> generateColors(int n, const QColor &background);

public Q_SLOTS:
    void rehighlight();
    void reformatBlocks(int from, int charsRemoved, int charsAdded);

protected:
    virtual void highlightBlock(const QString &text);
    void setFormatWithSpaces(const QString &text, int start, int count,
                             const QTextCharFormat &format);
    QTextCharFormat whitespacified(const QTextCharFormat &fmt);
    void setFormat(int start, int count, const QTextCharFormat &format);

private:
    BaseHighlighterPrivate *const d;
    friend class BaseHighlighterPrivate;
};

#endif   // BASEHIGHLIGHTER_H
