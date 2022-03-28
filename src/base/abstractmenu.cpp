#include "abstractmenu.h"

#include <QMenu>
#include <QDebug>

class AbstractMenuPrivate
{
    friend class AbstractMenu;
    QMenu *menu;
};

AbstractMenu::AbstractMenu(void *qMenu)
    : d(new AbstractMenuPrivate())
{
    if (!qMenu) {
        qCritical() << "Failed, use QAction(0x0) to AbstractAction";
        abort();
    }

    d->menu = (QMenu*)(qMenu);
    QMenu::connect(d->menu, &QMenu::destroyed,
                   d->menu, [this]() {
        delete this;
    },Qt::UniqueConnection);
}

AbstractMenu::~AbstractMenu()
{
    if (d) {
        delete d;
    }
}

void *AbstractMenu::qMenu()
{
    return d->menu;
}
