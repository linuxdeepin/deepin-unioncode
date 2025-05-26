// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMARTUTMANAGER_H
#define SMARTUTMANAGER_H

#include "common/itemnode.h"

#include "common/settings/settings.h"
#include "base/ai/abstractllm.h"

constexpr char kGeneralGroup[] { "General" };
constexpr char kActiveGroup[] { "ActiveSettings" };

constexpr char kPrompts[] { "Prompts" };
constexpr char kTestFrameworks[] { "TestFrameworks" };
constexpr char kTemplates[] { "Templates" };
constexpr char kNameFormat[] { "NameFormat" };
constexpr char kActivePrompt[] { "ActivePrompt" };
constexpr char kActiveTemplate[] { "ActiveTemplate" };
constexpr char kActiveTestFramework[] { "ActiveTestFramework" };

class SmartUTManagerPrivate;
class SmartUTManager : public QObject
{
    Q_OBJECT
public:
    static SmartUTManager *instance();

    Settings *utSetting();
    QStringList modelList() const;
    QList<dpfservice::ProjectInfo> projectList() const;
    AbstractLLM *findModel(const QString &model);
    QString userPrompt(const QString &framework) const;
    QString lastError() const;

public Q_SLOTS:
    void generateUTFiles(const QString &model,
                         NodeItem *item,
                         std::function<bool(NodeItem *item)> checkItemValid);
    void stop();
    void stop(NodeItem *item);

Q_SIGNALS:
    void itemStateChanged(NodeItem *item);

private:
    SmartUTManager(QObject *parent = nullptr);
    ~SmartUTManager();

    SmartUTManagerPrivate *const d;
};

#endif   // SMARTUTMANAGER_H
