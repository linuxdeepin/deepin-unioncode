#include "windowswitcher.h"
#include "common/util/eventdefinitions.h"

#include <QGridLayout>
#include <QActionGroup>

void setNavActionChecked(const QString ActionName, bool checked)
{
    if (navActionGroup->actions().size() > 0) {
        for (auto action : navActionGroup->actions()) {
            if (action->text() == ActionName){
                action->setChecked(checked);
            }
        }
    }
}

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
    qInfo() << __FUNCTION__;
    for (const QString &navName : centrals.keys()) {
        QString data = event.data().toString();
        QWidget *widget = centrals.value(navName);
        if (!widget)
            continue;

        if (data.contains(navName) && data.contains("Show")) {
            window->takeCentralWidget();
            window->setCentralWidget(widget);
            setNavActionChecked(navName, true);
            widget->show();
        } else {
            centrals.value(navName)->hide();
        }
    }
    qInfo() << event;
}
