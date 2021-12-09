#ifndef NAVEDITWIDGET_H
#define NAVEDITWIDGET_H

#include <QSplitter>

class QTabWidget;
class AbstractWidget;
class NavEditWidget final : public QSplitter
{
    Q_OBJECT
public:
    explicit NavEditWidget(QWidget *parent = nullptr);

public slots:
    void setTreeWidget(AbstractWidget *treeWidget);
    void setConsole(AbstractWidget *console);
    void addContextWidget(const QString &title, AbstractWidget *contextWidget);

private:
    void createCodeWidget(QSplitter *splitter);
    void createContextWidget(QSplitter *splitter);
    QTabWidget *tabWidget;
    QSplitter *verSplitter;
};

#endif // NAVEDITWIDGET_H
