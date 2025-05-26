// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODECOMPLETIONDELEGATE_H
#define CODECOMPLETIONDELEGATE_H

#include <QStyledItemDelegate>

class CodeCompletionView;
class CodeCompletionDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CodeCompletionDelegate(QObject *parent = nullptr);

    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    CodeCompletionView *view() const;
    void paintItemText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif   // CODECOMPLETIONDELEGATE_H
