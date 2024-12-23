// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERATEINPUT_H
#define GENERATEINPUT_H

#include <QWidget>

class GenerateInputPrivate;
class GenerateInput : public QWidget
{
    Q_OBJECT
public:
    explicit GenerateInput(QWidget *parent = nullptr);
    ~GenerateInput();

signals:
    void commandGenerated(const QString &command);

public slots:
    void onGenerate();
    void switchState(bool generating);

private:
    void initUi();
    void initLLM();
    void initConnect();
    GenerateInputPrivate *d;
};

#endif   // GENERATEINPUT_H
