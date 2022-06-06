#ifndef PROJECTRECEIVER_H
#define PROJECTRECEIVER_H

#include "services/project/projectservice.h"

#include <framework/framework.h>

class QStandardItem;
class SymbolCoreReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<SymbolCoreReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<SymbolCoreReceiver>;
public:
    explicit SymbolCoreReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;

    void projectEvent(const dpf::Event& event);
};

class ProjectProxy : public QObject
{
    Q_OBJECT
    ProjectProxy(){}
    ProjectProxy(const ProjectProxy&) = delete;

public:
    static ProjectProxy* instance() {
        static ProjectProxy ins;
        return &ins;
    }

signals:
    void toSetSymbolRootItem(const dpfservice::ProjectInfo &info);
};

#endif // PROJECTRECEIVER_H
