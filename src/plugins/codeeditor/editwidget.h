#ifndef EDITWIDGET_H
#define EDITWIDGET_H

#include <QWidget>

class EditWidgetPrivate;
class EditWidget : public QWidget
{
    Q_OBJECT
    EditWidgetPrivate *const d;
public:
    explicit EditWidget(QWidget *parent = nullptr);
    virtual ~EditWidget();

public slots:
    void openFile(const QString &filePath, const QString &workspaceFolder);
    void closeFile(const QString &filePath);

private:
    int tabIndex(const QString &tabTooltip);

private slots:
    void setDefaultFileEdit();
    void hideFileEdit(int tabIndex);
    void showFileEdit(int tabIndex);
    void hideFileStatusBar(int tabIndex);
    void showFileStatusBar(int tabIndex);
    void removeFileEdit(int tabIndex);
    void removeFileTab(int tabIndex);
};

#endif // EDITWIDGET_H
