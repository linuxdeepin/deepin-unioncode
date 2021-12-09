#include "abstractmenu.h"

#include <QMenu>
#include <QDebug>

const std::string MENU_FILE { QMenu::tr("File").toStdString() };
const std::string MENU_BUILD { QMenu::tr("Build").toStdString() };
const std::string MENU_DEBUG { QMenu::tr("Debug").toStdString() };
const std::string MENU_TOOLS { QMenu::tr("Tools").toStdString() };
const std::string MENU_HELP { QMenu::tr("Help").toStdString() };
const std::string MENU_CODETREE { QMenu::tr("CodeTree").toStdString() };

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
