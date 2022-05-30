#ifndef FINDTOOLBAR_H
#define FINDTOOLBAR_H

#include <QWidget>

class CurrentDocumentFind;
class FindToolBarPrivate;
class FindToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit FindToolBar(CurrentDocumentFind *currentDocumentFind);
    virtual ~FindToolBar();

signals:
    void advanced();

private:
    void setupUi();
    void findPrevious();
    void findNext();
    void advancedSearch();
    void replace();
    void replaceSearch();
    void replaceAll();

    FindToolBarPrivate *const d = nullptr;

};

#endif // FINDTOOLBAR_H
