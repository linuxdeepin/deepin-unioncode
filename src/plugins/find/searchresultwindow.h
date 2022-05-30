#ifndef SEARCHRESULTWINDOW_H
#define SEARCHRESULTWINDOW_H

#include <QWidget>
#include <QTreeView>

class SearchResultTreeView : public QTreeView
{

};

class SearchResultWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResultWindow(QWidget *parent = nullptr);

signals:
    void back();

private:
    void setupUi();
    void research();
};

#endif // SEARCHRESULTWINDOW_H
