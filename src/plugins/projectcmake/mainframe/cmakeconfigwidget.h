#ifndef CMAKECONFIGDIALOG_H
#define CMAKECONFIGDIALOG_H

#include <QWidget>

class CMakeConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CMakeConfigWidget(QWidget *parent = nullptr);
};

#endif // CMAKECONFIGDIALOG_H
