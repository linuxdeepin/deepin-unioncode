#ifndef NAVEDITMAINWINDOW_H
#define NAVEDITMAINWINDOW_H

#include <QMainWindow>

class QGridLayout;
class QTabWidget;
class AbstractCentral;
class AbstractWidget;
class AbstractConsole;
class QDockWidget;
class NavEditMainWindow : public QMainWindow
{
    Q_OBJECT
    QDockWidget *qTreeWidgetDock{nullptr};
    QDockWidget *qTabWidgetDock{nullptr};
    QDockWidget *qWatchWidgetDock{nullptr};
    QWidget *qTreeWidget{nullptr};
    QWidget *qEditWidget{nullptr};
    QWidget *qWatchWidget{nullptr};
    QTabWidget *qTabWidget{nullptr};
public:
    static NavEditMainWindow *instance();
    explicit NavEditMainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~NavEditMainWindow();
    void setConsole(AbstractConsole *console);
    void setTreeWidget(AbstractWidget *treeWidget);
    void setEditWidget(AbstractCentral *editWidget);
    void setWatchWidget(AbstractWidget *watchWidget);
    void addContextWidget(const QString &title, AbstractWidget *contextWidget);
    bool switchContextWidget(const QString &title);

protected:
    bool eventFilter(QObject *obj, QEvent *e);
};

#endif // NAVEDITMAINWINDOW_H
