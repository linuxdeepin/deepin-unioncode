#include "abstractaction.h"

#include <QDebug>
#include <QAction>

const std::string NAVACTION_RECENT { QAction::tr("Recent").toStdString() };
const std::string NAVACTION_EDIT { QAction::tr("Edit").toStdString() };
const std::string NAVACTION_DEBUG { QAction::tr("Debug").toStdString() };
const std::string NAVACTION_RUNTIME { QAction::tr("Runtime").toStdString() };

class AbstractActionPrivate
{
    friend class AbstractAction;
    QAction *action = nullptr;
};

AbstractAction::AbstractAction(void *qAction)
    : d(new AbstractActionPrivate())
{
    if (!qAction) {
        qCritical() << "Failed, use QAction(0x0) to AbstractAction";
        abort();
    }

    d->action = (QAction*)qAction;
    d->action->setObjectName("AbstractAction");
    QAction::connect(d->action, &QAction::destroyed,
                     d->action, [this]() {
        delete this;
    }, Qt::UniqueConnection);
}

AbstractAction::~AbstractAction()
{
    if (d) {
        delete d;
    }
}

void *AbstractAction::qAction()
{
    return(void*)(d->action);
}

