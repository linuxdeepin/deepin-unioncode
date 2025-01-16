// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include "common/widget/pagewidget.h"
#include "base/baseitemdelegate.h"
#include "services/ai/aiservice.h"

#include <DStyledItemDelegate>

#include <QAbstractTableModel>
#include <QPainter>

class TwoLineDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate {
public:
    TwoLineDelegate(QAbstractItemView *parent = nullptr) : DStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        DStyledItemDelegate::paint(painter, opt, index);

        painter->save();

        QString leftText = index.data(Qt::UserRole + 1).toString();
        QString rightText = index.data(Qt::UserRole + 2).toString();

        int textHeight = painter->fontMetrics().height();
        int verticalCenter = option.rect.top() + (option.rect.height() - textHeight) / 2;

        QRect leftTextRect(option.rect.left() + 10, verticalCenter, option.rect.width() / 2 - 15, textHeight);
        painter->drawText(leftTextRect, Qt::AlignLeft | Qt::AlignVCenter, leftText);

        QRect rightTextRect(option.rect.left() + option.rect.width() / 2 + 5, verticalCenter, option.rect.width() / 2 - 15, textHeight);
        painter->drawText(rightTextRect, Qt::AlignRight | Qt::AlignVCenter, rightText);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override { return QSize(option.rect.width(), 36);}
};

class LLMModels : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit LLMModels(QObject *parent = nullptr);
    ~LLMModels();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void appendLLM(const LLMInfo &info);
    void removeLLM(const LLMInfo &info);
    QList<LLMInfo> allLLMs();

private:
    QList<LLMInfo> LLMs;
};

class DetailWidgetPrivate;
class DetailWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit DetailWidget(QWidget *parent = nullptr);
    ~DetailWidget() override;

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

private:
    void setupUi();

    void addDefaultLLM(); // codegeex-chat-pro / codegeex-4

    bool getControlValue(QMap<QString, QVariant> &map);
    void setControlValue(const QMap<QString, QVariant> &map);

    DetailWidgetPrivate *const d;
};

#endif // DETAILWIDGET_H
