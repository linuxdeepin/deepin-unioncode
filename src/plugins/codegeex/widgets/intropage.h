// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTROPAGE_H
#define INTROPAGE_H

#include <DWidget>
#include <DLabel>
#include <QList>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

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
