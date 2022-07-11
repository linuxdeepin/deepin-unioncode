#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QProgressBar>
#include <QTextBrowser>
#include <QVBoxLayout>

class ProcessDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProcessDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setProgram(const QString & program);
    QString program() const;
    void setArguments(const QStringList &args);
    QStringList arguments();
    void setWorkDirectory(const QString &workDir);
    QString workDirectory() const;

protected:
    virtual void doShowRequestError(const QByteArray &array);
    virtual void doShowRequestOutput(const QByteArray &array);
    virtual void doRequestFinished(int exitCode, QProcess::ExitStatus status);
    virtual void doShowProgress(int current, int count);
    virtual void showEvent(QShowEvent *event) override;

protected:
    QProcess process;
    QProgressBar *progressBar{nullptr};
    QTextBrowser *textBrowser{nullptr};
    QVBoxLayout *vLayout{nullptr};
};

#endif // PROCESSDIALOG_H
