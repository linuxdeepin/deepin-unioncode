#include "windowswitcher.h"

#include <QGridLayout>

WindowSwitcher::WindowSwitcher(QObject *parent)
    : dpf::EventHandler (parent)
{

}

dpf::EventHandler::Type WindowSwitcher::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList WindowSwitcher::topics()
{
    return QStringList() << "Nav";
}

void WindowSwitcher::eventProcess(const dpf::Event &event)
{
    if (!WindowSwitcher::topics().contains(event.topic())) {
        qCritical() << "Fatal error, unsubscribed message received";
        abort();
    }

    if (event.topic() == "Nav")
        navEvent(event);
}

void WindowSwitcher::navEvent(const dpf::Event &event)
{
    qInfo() << __FUNCTION__ << this;
    for (const QString &navName : centrals.keys()) {
        if (event.data() == "Edit.Show") {
            QWidget *widget = centrals.value(navName);
            window->setCentralWidget(widget);
            widget->show();
        } else {
            centrals.value(navName)->hide();
        }
    }
    qInfo() << event;
}
