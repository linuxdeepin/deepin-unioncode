// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONSERVICE_H
#define OPTIONSERVICE_H

#include "optiongenerator.h"

#include "base/abstractwidget.h"

#include "framework/framework.h"

namespace dpfservice {

class OptionService final : public dpf::PluginService,
        dpf::AutoServiceRegister<OptionService>,
        dpf::QtClassFactory<OptionGenerator>,
        dpf::QtClassManager<OptionGenerator>
{
    Q_OBJECT
    Q_DISABLE_COPY(OptionService)
    typedef dpf::QtClassManager<OptionGenerator> GeneratorOptManager;
    typedef dpf::QtClassFactory<OptionGenerator> GeneratorOptFactory;

    QMap<QString, QStringList> groupGeneratorsMap;
public:
    inline static QString name()
    {
        return "org.deepin.service.OptionService";
    }

    explicit OptionService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    /*!
     * \brief supportGenerator 獲取支持的工程名稱
     * \return
     */
    template<class T>
    QStringList supportGeneratorName()
    {
        if (std::is_same<OptionGenerator, T>())
            return GeneratorOptFactory::createKeys();
        return {};
    }

    /*!
     * \brief supportGenerator 獲取支持的工程名稱
     * \param group
     * \return
     */
    template<class T>
    QStringList supportGeneratorNameByGroup(const QString &group)
    {
        QStringList allGenerators {};
        if (std::is_same<OptionGenerator, T>())
            allGenerators = GeneratorOptFactory::createKeys();
        if (!allGenerators.isEmpty()) {
            QStringList ret {};
            for (auto Generator : allGenerators) {
                if (groupGeneratorsMap.contains(group) && groupGeneratorsMap[group].contains(Generator))
                    ret.append(Generator);
            }
            return ret;
        }

        return {};
    }

    QStringList generatorGroups()
    {
        return groupGeneratorsMap.keys();
    }
    /*!
     * \brief implGenerator 導入工程文件生成器對象 T = Project::Generator類泛化對象
     * \param group
     * \param name 生成器對象名稱(唯一鍵)
     * \param errorString 錯誤信息
     */
    template<class T>
    bool implGenerator(const QString &group, const QString &name, QString *errorString = nullptr)
    {
        bool ret = false;
        if (std::is_base_of<OptionGenerator, T>()) {
            ret = GeneratorOptFactory::regClass<T>(name, errorString);
            if (ret)
                groupGeneratorsMap[group].append(name);
        }
        return false;
    }

    /*!
     * \brief createGenerator
     * \param name 名稱
     * \param errorString 錯誤信息
     * \return 生成器對象實例
     */
    template<class T>
    T *createGenerator(const QString &name, QString *errorString = nullptr)
    {
        if (std::is_base_of<OptionGenerator, T>()) {
            Generator *generator = GeneratorOptManager::value(name);
            if (!generator) {
                generator = GeneratorOptFactory::create(name, errorString);
                if (generator)
                    GeneratorOptManager::append(name, dynamic_cast<OptionGenerator*>(generator));
            }
            return dynamic_cast<T*>(generator);
        }
        return nullptr;
    }

    /**
     * @brief DPF_INTERFACE showOptionDialog
     * @param itemName will scroll to item with itemName, if empty, keep at top.
     */
    DPF_INTERFACE(void, showOptionDialog, const QString &itemName);
};
} //namespace dpfservice

#endif // OPTIONSERVICE_H
