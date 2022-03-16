#ifndef REFACTORWIDGET_H
#define REFACTORWIDGET_H

#include "common/common.h"
#include <QWidget>

class RefactorWidgetPrivate;
class RefactorWidget : public QWidget
{
    Q_OBJECT
    RefactorWidgetPrivate * const d;
public:
    static RefactorWidget *instance();
    explicit RefactorWidget(QWidget *parent = nullptr);
    virtual ~RefactorWidget();
    void displayReference(const lsp::References &data);

signals:
    void doubleClicked(const QString &filePath, const lsp::Range range);
};

#endif // REFACTORWIDGET_H
