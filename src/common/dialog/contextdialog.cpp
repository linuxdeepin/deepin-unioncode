#include "contextdialog.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QFile>

void ContextDialog::okCancel(QString text, QString title,
                             QMessageBox::Icon icon,
                             std::function<void (bool)> okCallBack,
                             std::function<void (bool)> cancelCallBack)
{
    QMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
    if (okCallBack)
        QObject::connect(messageBox.button(QMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    if (cancelCallBack)
        QObject::connect(messageBox.button(QMessageBox::Cancel),
                         &QAbstractButton::clicked, cancelCallBack);
    messageBox.exec();
}

void ContextDialog::ok(QString text, QString title,
                       QMessageBox::Icon icon,
                       std::function<void (bool)> okCallBack)
{
    QMessageBox messageBox;
    messageBox.setWindowTitle(title);
    messageBox.setText(text);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(QMessageBox::Ok);
    if (okCallBack)
        QObject::connect(messageBox.button(QMessageBox::Ok),
                         &QAbstractButton::clicked, okCallBack);
    messageBox.exec();
}
