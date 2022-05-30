#ifndef FINDTOOLWINDOW_H
#define FINDTOOLWINDOW_H

#include <QWidget>

class FindToolWindowPrivate;
class FindToolWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FindToolWindow(QWidget *parent = nullptr);

signals:

private:
    void setupUi();
    void search();
    void replace();
    void addSearchParamWidget(QWidget *parentWidget);
    void addSearchResultWidget(QWidget *parentWidget);
    void switchSearchParamWidget();


    FindToolWindowPrivate *const d;
};

#endif // FINDTOOLWINDOW_H
