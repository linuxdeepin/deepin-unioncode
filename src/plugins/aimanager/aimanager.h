// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/ai/abstractllm.h"
#include "services/ai/aiservice.h"

#include <QObject>

// option manager -> custom models
static const char *kCATEGORY_CUSTOMMODELS = "CustomModels";
static const char *kCATEGORY_OPTIONKEY = "OptionKey";

class AiManagerPrivate;
class AiManager : public QObject
{
    Q_OBJECT
public:
    static AiManager *instance();
    ~AiManager();

    QList<LLMInfo> getAllModel();
    AbstractLLM *getLLM(const LLMInfo &info);

    void appendModel(const LLMInfo &LLMInfo);
    void removeModel(const LLMInfo &LLMInfo);
    bool checkModelValid(const LLMInfo &info, QString *errStr);

    void readLLMFromOption();

private:
    AiManager(QObject *parent = nullptr);
    AiManagerPrivate *d;
};
