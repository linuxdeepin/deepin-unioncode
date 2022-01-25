#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <QDialog>

class ProcessDialogPrivate;
class ProcessDialog : public QDialog
{
    Q_OBJECT
    ProcessDialogPrivate *const d;
public:
    explicit ProcessDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~ProcessDialog();
    static ProcessDialog* globalInstance();
    void setRunning(bool runable);
    ProcessDialog& operator << (const QString &message);
    void setTitle(const QString &title);
};

#endif // PROCESSDIALOG_H
