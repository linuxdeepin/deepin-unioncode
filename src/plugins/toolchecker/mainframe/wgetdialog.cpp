#include "wgetdialog.h"

#include <QRegularExpression>
#include <QDebug>

WGetDialog::WGetDialog(QWidget *parent, Qt::WindowFlags f)
    : ProcessDialog (parent, f)
{
    setWindowTitle("wget request download");
    process.setProgram("wget");
}

void WGetDialog::setWorkDirectory(const QString &workDir)
{
    process.setWorkingDirectory(workDir);
}

QString WGetDialog::workDirectory() const
{
    return process.workingDirectory();
}

void WGetDialog::setWgetArguments(const QStringList &list)
{
    setWindowTitle(windowTitle() + " " + list.join(" "));
    process.setArguments(list);
}

QStringList WGetDialog::wgetArguments() const
{
    return process.arguments();
}

void WGetDialog::doShowRequestError(const QByteArray &array)
{
    static QString cacheData;
    static QString headTitle;
    static QString downloadLine;
    cacheData += array;
    auto datas = cacheData.split("\n\n");
    if (datas.size() >= 2) {
        headTitle = datas.takeAt(0);
        textBrowser->setText(headTitle + "\n" + downloadLine);
        cacheData = datas.join("");
    }

    if (!headTitle.isEmpty()) {
        auto lines = cacheData.split("\n");
        if (lines.size() >= 2) {
            downloadLine = lines.takeAt(0);
            textBrowser->setText(headTitle + "\n" + downloadLine);
            QRegularExpression regExp("\\d+\\%");
            auto matchRes = regExp.match(downloadLine);
            if (matchRes.hasMatch()) {
                QString matched = matchRes.captured();
                matched = matched.remove(matched.size() - 1, 1);
                doShowProgress(matched.toInt(), 100);
            }
            cacheData = lines.join("");
        }
    }
}

void WGetDialog::doShowRequestOutput(const QByteArray &array)
{
    textBrowser->append(array);
}

void WGetDialog::doRequestFinished(int exitCode, QProcess::ExitStatus status)
{
    this->close();
    qInfo() << exitCode << status;
}

void WGetDialog::showEvent(QShowEvent *event)
{
    process.start();
    return QDialog::showEvent(event);
}
