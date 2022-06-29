/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PROJECTSERVICE_H
#define PROJECTSERVICE_H

#include "projectgenerator.h"
#include "projectinfo.h"
#include "projectviewinterface.h"
#include "symbolgenerator.h"
#include "symbolinfo.h"
#include "symbolviewinterface.h"

#include <QTabWidget>

namespace dpfservice {

struct Target {
    QString name;
    QString path;
    QString targetID;

    QString buildCommand;
    QStringList buildArguments;
    QString buildTarget;
    bool stopOnError = false;
    bool useDefaultCommand = false;

    QString outputPath;

    // TODO(mozart):tempory
    bool enableEnv;

    bool operator==(const Target &other) const
    {
        if (name == other.name
                && path == other.path)
            return true;

        return false;
    }
};
using Targets = QVector<Target>;

enum TargetType {
    kBuildTarget,
    kRebuildTarget,
    kCleanTarget,
    kActiveExecTarget
};

class ProjectService final : public dpf::PluginService,
        dpf::AutoServiceRegister<ProjectService>,
        dpf::QtClassFactory<ProjectGenerator>,
        dpf::QtClassManager<ProjectGenerator>,
        dpf::QtClassFactory<SymbolGenerator>,
        dpf::QtClassManager<SymbolGenerator>
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectService)
    typedef dpf::QtClassManager<ProjectGenerator> GeneratorProManager;
    typedef dpf::QtClassFactory<ProjectGenerator> GeneratorProFactory;
    typedef dpf::QtClassManager<SymbolGenerator> GeneratorSymManager;
    typedef dpf::QtClassFactory<SymbolGenerator> GeneratorSymFactory;
public:
    static QString name()
    {
        return "org.deepin.service.ProjectService";
    }

    explicit ProjectService(QObject *parent = nullptr)
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
        if (std::is_same<ProjectGenerator, T>())
            return GeneratorProFactory::createKeys();
        else if (std::is_same<SymbolGenerator, T>())
            return GeneratorProFactory::createKeys();
        else {
            qCritical() << "must SymbolGenerator or ProjectGenerator, "
                        << "not support " << typeid (T).name();
            abort();
        }
    }

    /*!
     * \brief implGenerator 導入工程文件生成器對象 T = Project::Generator類泛化對象
     * \param name 生成器對象名稱(唯一鍵)
     * \param errorString 錯誤信息
     */
    template<class T>
    bool implGenerator(const QString &name, QString *errorString = nullptr)
    {
        if (std::is_base_of<ProjectGenerator, T>())
            return GeneratorProFactory::regClass<T>(name, errorString);
        else if (std::is_base_of<SymbolGenerator, T>())
            return GeneratorSymFactory::regClass<T>(name, errorString);
        else {
            qCritical() << "must base class SymbolGenerator or ProjectGenerator, "
                        << "not support " << typeid (T).name();
            abort();
        }
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
        if (std::is_base_of<ProjectGenerator, T>()) {
            Generator *generator = GeneratorProManager::value(name);
            if (!generator) {
                generator = GeneratorProFactory::create(name, errorString);
                if (generator)
                    GeneratorProManager::append(name, dynamic_cast<ProjectGenerator*>(generator));
            }
            return dynamic_cast<T*>(generator);
        } else if (std::is_base_of<SymbolGenerator, T>()) {
            Generator *generator = GeneratorSymManager::value(name);
            if (!generator) {
                generator = GeneratorSymFactory::create(name, errorString);
                if (generator)
                    GeneratorSymManager::append(name, dynamic_cast<SymbolGenerator*>(generator));
            }
            return dynamic_cast<T*>(generator);
        } else {
            qCritical() << "must base class SymbolGenerator or ProjectGenerator, "
                        << "not support "<< typeid (T).name();
            abort();
        }
    }

    /*!
     * \brief name
     */
    template<class T>
    QString name(T* value) const{
        return QtClassManager<T>::key(value);
    }

    template<class T>
    QList<T*> values() const {
        return QtClassManager<T>::values();
    }

    /*!
     * \brief projectView 工程视图接口对象
     */
    ProjectViewInterface projectView;

    /*!
     * \brief symbolView 符号视图接口对象
     */
    SymbolViewInterface symbolView;

    /*!
     * \brief getActiveTarget
     * \param TargetType
     * \return Target
     */
    DPF_INTERFACE(Target, getActiveTarget, TargetType);

Q_SIGNALS:
    /*!
     * \brief projectConfigureDone
     */
    void projectConfigureDone();
};

/* MainWindow codeediter workspace title,
 * use in window service swtich workspace
 */
inline const QString MWCWT_PROJECTS {QTabWidget::tr("Projects")};
inline const QString MWCWT_SYMBOL {QTabWidget::tr("Symbol")};

} //namespace dpfservice

#endif // PROJECTSERVICE_H
