#ifndef WINDOWKEEPER_H
#define WINDOWKEEPER_H

#include "base/abstractnav.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"
#include "base/abstractmenubar.h"
#include "base/abstractmainwindow.h"
#include "base/abstractcentral.h"

#include <framework/framework.h>

#include <QObject>

class QAction;
class QMainWindow;
class QStatusBar;
class QMenuBar;
class QToolBar;
class QActionGroup;

extern QHash<QString, QWidget *> centrals;
extern QMainWindow *window;
extern QActionGroup *navActionGroup;
extern QToolBar *toolbar;
class WindowKeeper final :public QObject
{
    Q_OBJECT
    friend class WindowSwitcher;

public:
    explicit WindowKeeper(QObject *parent = nullptr);

public slots:
    void addNavAction(AbstractAction *action);
    void addCentral(const QString &navName, AbstractCentral *central);
    void addMenu(AbstractMenu *menu);
    void addAction(const QString &menuName, AbstractAction *action);
    void initUserWidget();

private :
    void layoutWindow(QMainWindow *window);
    void createNavRuntime(QToolBar *toolbar);
    void createNavDebug(QToolBar *toolbar);
    void createNavEdit(QToolBar *toolbar);
    void createNavRecent(QToolBar *toolbar);
    void createStatusBar(QMainWindow *window);
    void createHelpActions(QMenuBar *menuBar);
    void createToolsActions(QMenuBar *menuBar);
    void createDebugActions(QMenuBar *menuBar);
    void createBuildActions(QMenuBar *menuBar);
    void createFileActions(QMenuBar *menuBar);
};

#endif // WINDOWKEEPER_H
