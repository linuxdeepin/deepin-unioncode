#ifndef GITQLIENTWIDGET_H
#define GITQLIENTWIDGET_H

#include "GitQlient.h"
#include "QLogger.h"

class GitQlientWidget : public GitQlient
{
    Q_OBJECT
public:
    explicit GitQlientWidget(QWidget *parent = nullptr);
};

#endif // GITQLIENTWIDGET_H
