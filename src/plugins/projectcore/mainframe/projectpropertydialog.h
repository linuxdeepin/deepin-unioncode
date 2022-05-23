#ifndef PROJECTPROPERTYDIALOG_H
#define PROJECTPROPERTYDIALOG_H

#include <QTextBrowser>
#include <QDialog>
#include <QVBoxLayout>

class ProjectPropertyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectPropertyDialog(QWidget *parent = nullptr,
                                   Qt::WindowFlags f = Qt::WindowFlags());
    void setPropertyText(const QString &text);
private:
    QVBoxLayout *vLayout {nullptr};
    QTextBrowser *textBrowser {nullptr};
};

#endif // PROJECTPROPERTYDIALOG_H
