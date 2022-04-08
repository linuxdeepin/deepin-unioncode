#ifndef PROJECTRECEIVER_H
#define PROJECTRECEIVER_H

#include <framework/framework.h>

class ProjectReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<ProjectReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ProjectReceiver>;
public:
    explicit ProjectReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

class ProjectProxy : public QObject
{
    Q_OBJECT
    ProjectProxy(){}
    ProjectProxy(const ProjectProxy&) = delete;

public:
    static ProjectProxy* instance()
    {
        static ProjectProxy ins;
        return &ins;
    }
signals:

};

#endif // PROJECTRECEIVER_H
