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

    /*!
     * \brief projectView 工程视图接口对象
     */
    ProjectViewInterface projectView;

    /*!
     * \brief symbolView 符号视图接口对象
     */
    SymbolViewInterface symbolView;

    /*!
     * \brief DPF_INTERFACE
     * insert one item to option dialog with item name and widget.
     */
    DPF_INTERFACE(bool, insertOptionPanel, const QString &itemName, QWidget *panel);

    /*!
     * \brief showProjectOptionsDlg
     * \param show options dialog at panel witch
     */
    DPF_INTERFACE(void, showProjectOptionsDlg, const QString &itemName, const QString &tabName);

    /**
     * @brief showConfigureProjDlg
     * @param outOutputPath return output path
     * @param outBuildTypes return build types user selected
     */
    DPF_INTERFACE(void, showConfigureProjDlg, QString &projectPath);

    /**
     * @brief getDefaultOutputPath
     * @param projectPath
     */
    DPF_INTERFACE(QString, getDefaultOutputPath);

Q_SIGNALS:
    /*!
     * \brief targetExecute 工程目标执行指令, 调用构建系统的入口
     *  当前projectservice不属于构建系统的一部分，需要构建系统支持
     * \param program 程序
     * \param arguments 程序参数
     */
    void targetCommand(const QString &program, const QStringList &arguments);

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
