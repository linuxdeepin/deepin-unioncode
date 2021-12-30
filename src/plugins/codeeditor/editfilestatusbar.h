#ifndef EDITFILESTATUSBAR_H
#define EDITFILESTATUSBAR_H

#include <QStatusBar>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include <iostream>
#include <functional>

class EditFileStatusBarPrivate;
class EditFileStatusBar : public QStatusBar
{
    Q_OBJECT
    EditFileStatusBarPrivate * const d;

public:

    enum StandardButton
    {
        Reload,
        Cancel
    };

    explicit EditFileStatusBar(QWidget *parent = nullptr);
    virtual ~EditFileStatusBar();
    QPushButton * button(StandardButton button);
    static EditFileStatusBar* changedReload(const QString &filePath);
};

#endif // EDITFILESTATUSBAR_H
