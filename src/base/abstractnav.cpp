#include "abstractnav.h"
#include "abstractaction.h"

#include <QToolBar>
#include <QDebug>

class AbstractNavPrivate
{
    friend class AbstractNav;
    QToolBar* toolBar = nullptr;
    std::list<AbstractAction*> actions{};
};

AbstractNav::~AbstractNav()
{
    if (d) {
        delete d;
    }
}

void *AbstractNav::qToolbar()
{
    return (void*)(d->toolBar);
}

AbstractNav::AbstractNav(void *qToolBar)
    : d(new AbstractNavPrivate())
{
    if (!qToolBar) {
        qCritical() << "Failed, use QToolbar(0x0) to AbstractMenuBar";
        abort();
    }

    d->toolBar = (QToolBar*)(qToolBar);
    QObject::connect(d->toolBar, &QAction::destroyed,
                     d->toolBar, [this](){
        delete this;
    }, Qt::UniqueConnection);
}
