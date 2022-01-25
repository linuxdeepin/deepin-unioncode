#ifndef WORKSPACEOBJECT_H
#define WORKSPACEOBJECT_H

#include <framework/framework.h>

#include <QString>

class WorkspaceObject;
class WorkspaceObjectFactory : dpf::QtClassFactory<WorkspaceObject>,
        dpf::QtClassManager<WorkspaceObject>
{
    static WorkspaceObjectFactory *globalInstance();

    QString findkey(WorkspaceObject *ins);

public:
    template<class T>
    static bool regClass(const QString &name, QString *errorString = nullptr)
    {
        return globalInstance()->dpf::QtClassFactory<WorkspaceObject>
                ::regClass<T>(name, errorString);
    }

    static WorkspaceObject* create(const QString &name, QString *errorString = nullptr);

    static QString key(WorkspaceObject *ins);
};

class WorkspaceObject : public QObject
{
    Q_OBJECT
public:
    WorkspaceObject();
    virtual ~WorkspaceObject();
    static QString buildSystemName(){return "";}
    virtual bool generate(const QString &rootPath, const QString &targetPath);

signals:
    /*!
     * \brief generatePercent 过程百分比信号
     * \param value 百分比 1~100
     */
    void generatePercent(int value);

    /*!
     * \brief generateStart 生成开始信号
     * \param rootPath 工程根目录
     * \param targetPath 生成workspace目录
     */
    void generateStart(const QString &rootPath, const QString &targetPath);

    /*!
     * \brief generateEnd 生成结束信号
     * \param rootPath 工程根目录
     * \param targetPath 生成workspace目录
     */
    void generateEnd(const QString &rootPath, const QString &targetPath);
};

#endif // WORKSPACEOBJECT_H
