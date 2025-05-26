// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generateinput.h"
#include "services/ai/aiservice.h"

#include <DLineEdit>
#include <DSuggestButton>
#include <DSpinner>
#include <DIconButton>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class GenerateInputPrivate
{
public:
    DLineEdit *edit { nullptr };
    DSuggestButton *confirmBtn { nullptr };
    DIconButton *closeBtn { nullptr };
    DSpinner *spinner { nullptr };
    AbstractLLM *llm { nullptr };
};

GenerateInput::GenerateInput(QWidget *parent)
    : QWidget(parent), d(new GenerateInputPrivate)
{
    initUi();
    initLLM();
    initConnect();
}

GenerateInput::~GenerateInput()
{
    if (d)
        delete d;
}

void GenerateInput::initUi()
{
    setFixedHeight(50);
    auto layout = new QHBoxLayout(this);
    d->edit = new DLineEdit(this);
    d->edit->setPlaceholderText(tr("Describe the task you want to execute"));
    d->confirmBtn = new DSuggestButton(this);
    d->confirmBtn->setText(tr("Generate"));
    d->confirmBtn->setEnabled(false);
    d->closeBtn = new DIconButton(this);
    d->closeBtn->setIcon(QIcon::fromTheme("common_close"));
    d->closeBtn->setIconSize({ 16, 16 });
    d->closeBtn->setFlat(true);
    d->spinner = new DSpinner(this);
    d->spinner->setFixedSize(16, 16);
    d->spinner->hide();

    layout->addWidget(d->edit);
    layout->addWidget(d->confirmBtn);
    layout->addWidget(d->closeBtn);
}

void GenerateInput::initLLM()
{
    using namespace dpfservice;
    auto aiSrv = dpfGetService(AiService);
    LLMInfo liteModel;
    auto liteLLMInfo = aiSrv->getCodeGeeXLLMPro();
    d->llm = aiSrv->getLLM(liteLLMInfo);
    d->llm->setStream(false);
    connect(d->llm, &AbstractLLM::dataReceived, this, [=](const QString &data, AbstractLLM::ResponseState state){
        if (state == AbstractLLM::Failed) {
            QString err = "";
            auto valid = d->llm->checkValid(&err);
            if (!valid){
                switchState(false);
                emit commandGenerated(err);
            } else {
                emit commandGenerated(tr("Please try again later"));
            }
        }
        auto response = data;
        QString results;
        QRegularExpression regex(R"(```.*\n((.*\n)*?.*)\n\s*```)");

        QRegularExpressionMatchIterator matchIterator = regex.globalMatch(response);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            QString codePart = match.captured(1).trimmed();
            if (!codePart.isEmpty()) {
                if (!results.isEmpty())
                    results.append(';');
                results.append(codePart);
            }
        }

        switchState(false);
        emit commandGenerated(results);
    });
}

void GenerateInput::initConnect()
{
    connect(d->edit, &DLineEdit::returnPressed, this, &GenerateInput::onGenerate);
    connect(d->edit, &DLineEdit::textChanged, this, [=](){
        d->confirmBtn->setEnabled(!d->edit->text().isEmpty());
    });
    connect(d->confirmBtn, &DSuggestButton::clicked, this, &GenerateInput::onGenerate);
    connect(d->closeBtn, &DIconButton::clicked, this, [=](){
        this->hide();
    });
}

void GenerateInput::onGenerate()
{
    auto text = d->edit->text();
    if (text.isEmpty()) {
        switchState(false);
        return;
    }

    switchState(true);
    QString prompt = "你是一个智能终端机器人，你的工作环境是deepin-os/UOS/Linux，你的任务是根据描述生成可以直接使用的终端命令，不要进行额外的回答。描述是：" + text;
    d->llm->request(prompt);
}

void GenerateInput::switchState(bool generating)
{
    if (generating) {
        d->spinner->start();
        d->spinner->show();
        d->spinner->move(d->edit->rect().bottomRight() - QPoint(38, 16));
        d->edit->setEnabled(false);
        d->confirmBtn->setEnabled(false);
    } else {
        d->spinner->stop();
        d->spinner->hide();
        d->edit->clear();
        d->edit->setEnabled(true);
        d->confirmBtn->setEnabled(true);
    }
}
