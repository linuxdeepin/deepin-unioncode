// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AISERVICE_H
#define AISERVICE_H

#include "services/services_global.h"
#include "base/ai/abstractllm.h"
#include <framework/framework.h>

#include <QIODevice>
#include <QIcon>

enum LLMType {
    OPENAI,
    ZHIPU_CODEGEEX
};

static QString LLMTypeToString(LLMType type)
{
    switch (type) {
    case OPENAI:
        return "OpenAi";
    case ZHIPU_CODEGEEX:
        return QObject::tr("ZhiPu");
    default:
        return "UnKnown";
    }
}

struct LLMInfo
{
    QString modelName = "";
    QString modelPath = "";
    QString apikey = "";
    QIcon icon;
    LLMType type;
    bool isdefault = false;
    bool operator==(const LLMInfo &info) const
    {
        return (this->modelName == info.modelName
                && this->modelPath == info.modelPath
                && this->apikey == info.apikey
                && this->type == info.type);
    }
    QVariant toVariant() const
    {
        QVariantMap map;
        map["modelName"] = modelName;
        map["modelPath"] = modelPath;
        map["apikey"] = apikey;
        map["type"] = static_cast<int>(type);
        map["icon"] = icon.name();
        return QVariant(map);
    }
    static LLMInfo fromVariantMap(const QVariantMap &map)
    {
        LLMInfo info;
        info.modelName = map["modelName"].toString();
        info.modelPath = map["modelPath"].toString();
        info.apikey = map["apikey"].toString();
        info.type = static_cast<LLMType>(map["type"].toInt());
        info.icon = QIcon::fromTheme(map["icon"].toString());
        return info;
    }
};

Q_DECLARE_METATYPE(LLMType);

namespace dpfservice {
// service interface
class SERVICE_EXPORT AiService final : public dpf::PluginService, dpf::AutoServiceRegister<AiService>
{
    Q_OBJECT
    Q_DISABLE_COPY(AiService)
public:
    static QString name()
    {
        return "org.deepin.service.AiService";
    }

    explicit AiService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    // custom model
    DPF_INTERFACE(AbstractLLM *, getLLM, const LLMInfo &info);
    DPF_INTERFACE(QList<LLMInfo>, getAllModel);
    DPF_INTERFACE(bool, registerLLM, const QString &name, const LLMInfo &info);

    // Ai chat
    DPF_INTERFACE(void, chatWithAi, const QString &message);

    // default Model
    DPF_INTERFACE(LLMInfo, getCodeGeeXLLMLite);
    DPF_INTERFACE(LLMInfo, getCodeGeeXLLMPro);
    // rag
    DPF_INTERFACE(void, generateRag, const QString &projectPath);

     /*!
     * \brief
        Could be empty or not completed.  see obj.isEmpty or obj["Completed"].toBool
        May block, recommended to run in a separate thread
     * \param query . Find similar chunks based on this text as accurately as possible
     * \param topItems .  count of chunks
     * \return    
        JsonObject:
            Query: str
            Chunks: Arr[fileName:str, content:str, similarity:float]
            Completed: Bool
            Instructions: obj{name:str, description:str, content:str}
     */
    DPF_INTERFACE(QJsonObject, query, const QString &projectPath, const QString &query, int topItems);
};

}   // namespace dpfservice

#endif // AISERVICE_H
