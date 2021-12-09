#ifndef CONTEXTDIALOG_H
#define CONTEXTDIALOG_H

#include <QIcon>
#include <QString>
#include <QMessageBox>
#include <functional>

class ContextDialog final
{
    Q_DISABLE_COPY(ContextDialog)
    ContextDialog() = delete;
public:
    static void okCancel(QString text,
                         QString title = "Warning",
                         QMessageBox::Icon icon = QMessageBox::Warning,
                         std::function<void(bool)> okCallBack = nullptr,
                         std::function<void(bool)> cancelCallBack = nullptr);

    static void ok(QString text,
                   QString title = "Error",
                   QMessageBox::Icon icon = QMessageBox::Critical,
                   std::function<void(bool)> okCallBack = nullptr);
};

#endif
