// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractmenu.h"

#include <QDebug>
#include <QScopedPointer>  

// Clase privada para encapsular detalles de implementación
class AbstractMenuPrivate {
public:
    QScopedPointer<DMenu> menu; // Usar QScopedPointer para gestionar la vida del menú
    QList<AbstractAction*> actionList;
};

// Constructor
AbstractMenu::AbstractMenu(DMenu* qMenu)
    : d(new AbstractMenuPrivate)
{
    Q_ASSERT(qMenu);
    d->menu.reset(qMenu); // Tomar propiedad del menú usando QScopedPointer

    connect(d->menu.data(), &QObject::destroyed, this, &QObject::deleteLater); // Conexión optimizada
}

// Destructor
AbstractMenu::~AbstractMenu() = default;  // El destructor por defecto es suficiente gracias a QScopedPointer

// Getter
DMenu* AbstractMenu::qMenu() const 
{
    return d->menu.data();  
}

// Método para agregar una acción
void AbstractMenu::addAction(AbstractAction* action)
{
    d->actionList.append(action);
    d->menu->addAction(action->qAction());
}

// Método para obtener la lista de acciones
QList<AbstractAction*> AbstractMenu::actionList() const 
{
    return d->actionList;
}
