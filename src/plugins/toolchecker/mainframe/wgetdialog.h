#ifndef WGETDIALOG_H
#define WGETDIALOG_H

#include "processdialog.h"

class WGetDialog : public ProcessDialog
{
    Q_OBJECT
public:
    WGetDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setWorkDirectory(const QString &workDir);
    QString workDirectory() const;
    void setWgetArguments(const QStringList &list);
    QStringList wgetArguments() const;

protected:
    virtual void doShowRequestError(const QByteArray &array) override;
    virtual void doShowRequestOutput(const QByteArray &array) override;
    virtual void doRequestFinished(int exitCode, QProcess::ExitStatus status) override;
    virtual void showEvent(QShowEvent *event) override;
};

#endif // WGETDIALOG_H
