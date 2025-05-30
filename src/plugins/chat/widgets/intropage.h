// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTROPAGE_H
#define INTROPAGE_H

#include <DPushButton>
#include <DWidget>
#include <DLabel>

#include <QList>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

class SuggestButton : public DPushButton
{
    Q_OBJECT
public:
    explicit SuggestButton(const QString &text, const QString &iconName, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString originalText { "" };
    int minimumWidth = 0;
};

class IntroPage : public DWidget
{
    Q_OBJECT
public:
    explicit IntroPage(QWidget *parent = nullptr);

signals:
    void suggestionToSend(const QString &suggesstion);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();
    void initLogo();
    void initIntroContent();
    void initSuggestContent();

    void appendDescLabel(QVBoxLayout *layout, const QString &text);
    void appendSuggestButton(QVBoxLayout *layout, const QString &text, const QString &iconName = "");

    QList<DLabel *> labelToPaint {};
};

#endif   // INTROPAGE_H
