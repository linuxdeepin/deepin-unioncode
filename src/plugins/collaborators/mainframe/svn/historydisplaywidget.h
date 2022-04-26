#ifndef HISTORYDISPLAYWIDGET_H
#define HISTORYDISPLAYWIDGET_H

#include <QSplitter>

class HistoryLogWidget;
class HistoryDiffWidget;
class HistoryDisplayWidget : public QSplitter
{
    Q_OBJECT
public:
    explicit HistoryDisplayWidget(QWidget *parent = nullptr);
    HistoryLogWidget *logWidget();
    HistoryDiffWidget *diffWidget();

private:
    HistoryLogWidget *hisLogWidget{nullptr};
    HistoryDiffWidget *hisDiffWidget{nullptr};
};

#endif // HISTORYDISPLAYWIDGET_H
