#ifndef NAVEDITMAINWINDOW_H
#define NAVEDITMAINWINDOW_H

#include <QMainWindow>

class QGridLayout;
class QTabWidget;
class AbstractCentral;
class AbstractWidget;
class AbstractConsole;
class QDockWidget;
class AutoHideDockWidget;
class NavEditMainWindow : public QMainWindow
{
    Q_OBJECT
    AutoHideDockWidget *qDockWidgetWorkspace{nullptr};
    AutoHideDockWidget *qDockWidgetContext{nullptr};
    AutoHideDockWidget *qDockWidgetWatch{nullptr};
    AutoHideDockWidget *qDockWidgetFindToolBar{nullptr};
    QWidget *qWidgetEdit{nullptr};
    QWidget *qWidgetWatch{nullptr};
    QTabWidget *qTabWidgetContext{nullptr};
    QTabWidget *qTabWidgetWorkspace{nullptr};
public:
    static NavEditMainWindow *instance();
    explicit NavEditMainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~NavEditMainWindow();
    QStringList contextWidgetTitles() const;
    void setConsole(AbstractConsole *console);
    void addWidgetWorkspace(const QString &title, AbstractWidget *treeWidget);
    void setWidgetEdit(AbstractCentral *editWidget);
    void setWidgetWatch(AbstractWidget *watchWidget);
    void addWidgetContext(const QString &title, AbstractWidget *contextWidget);
    bool switchWidgetWorkspace(const QString &title);
    bool switchWidgetContext(const QString &title);
    void addFindToolBar(AbstractWidget *findToolbar);
    void showFindToolBar();
};

#endif // NAVEDITMAINWINDOW_H
