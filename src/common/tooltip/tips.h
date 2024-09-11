// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPS_H
#define TIPS_H

#include <QLabel>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

class TipLabel : public QLabel
{
public:
    enum ContentType {
        TextContent = 0,
        WidgetContent
    };

    explicit TipLabel(QWidget *parent = nullptr);

    virtual void setContent(const QVariant &content) = 0;
    virtual bool isInteractive() const { return false; }
    virtual int showTime() const = 0;
    virtual void configure(const QPoint &pos) = 0;
    virtual bool canHandleContentReplacement(ContentType type) const = 0;
    virtual bool equals(ContentType type, const QVariant &other) const = 0;
};

class TextTip : public TipLabel
{
public:
    explicit TextTip(QWidget *parent = nullptr);

    void setContent(const QVariant &content) override;
    bool isInteractive() const override;
    void configure(const QPoint &pos) override;
    bool canHandleContentReplacement(ContentType type) const override;
    int showTime() const override;
    bool equals(ContentType type, const QVariant &other) const override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    bool likelyContainsLink() const;

    QString tipText;
};

class WidgetTip : public TipLabel
{
    Q_OBJECT
public:
    explicit WidgetTip(QWidget *parent = nullptr);

    void setContent(const QVariant &content) override;
    void configure(const QPoint &pos) override;
    bool canHandleContentReplacement(ContentType type) const override;
    int showTime() const override { return 30000; }
    bool equals(ContentType type, const QVariant &other) const override;
    bool isInteractive() const override { return true; }

private:
    QWidget *widget { nullptr };
    QVBoxLayout *layout { nullptr };
};

#endif   // TIPS_H
