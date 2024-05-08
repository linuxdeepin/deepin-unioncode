// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITDIFFWIDGET_H
#define GITDIFFWIDGET_H

#include "gitbasewidget.h"
#include "giteditor.h"
#include "utils/diffutils.h"

class GitDiffEditor : public GitEditor
{
    Q_OBJECT
public:
    struct PaintEventBlockData
    {
        QRectF boundingRect;
        QVector<QTextLayout::FormatRange> selections;
        QTextLayout *layout = nullptr;
        int position = 0;
        int length = 0;
    };

    explicit GitDiffEditor(QWidget *parent = nullptr);

    void setSelections(const DiffSelections &selections) { diffSelections = selections; }
    void setDiffData(const SideDiffData &data) { diffData = data; }
    void updateTheme();

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    QString skippedText(int skippedNumber) const;
    void paintBlock(const QRect &eventRect) const;
    void paintBlock(QPainter *painter,
                    const QTextBlock &block,
                    const QPointF &offset,
                    const QVector<QTextLayout::FormatRange> &selections,
                    const QRect &clipRect) const;
    void paintSeparator(const QRect &eventRect) const;
    void paintSeparator(QPainter &painter, const QColor &color, const QString &text,
                        const QTextBlock &block, int top) const;

    DiffSelections diffSelections;
    SideDiffData diffData;
    QColor fileLineForeground;
    QColor chunkLineForeground;
};

class GitDiffWidgetPrivate;
class GitDiffWidget : public GitBaseWidget
{
    Q_OBJECT
public:
    explicit GitDiffWidget(QWidget *parent = nullptr);
    ~GitDiffWidget();

    void setGitInfo(const QStringList &infos) override;
    void setReadyMessage(const QString &msg) override;

public Q_SLOTS:
    void onParsePathFinished();
    void onParseFileDatasFinished(bool success);

Q_SIGNALS:
    void reqParsePatch(const QString &patch);
    void reqParseFileDatas(const QList<FileData> &dataList, const QFont &font);

private:
    GitDiffWidgetPrivate *const d;
};

#endif   // GITDIFFWIDGET_H
