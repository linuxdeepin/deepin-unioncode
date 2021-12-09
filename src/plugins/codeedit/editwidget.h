#ifndef EDITWIDGET_H
#define EDITWIDGET_H

//#include "3rdparty/qscintilla/src/Qsci/qsciscintilla.h"
#include <QTextEdit>

class EditWidget : public QTextEdit //: public QsciScintilla
{
public:
    explicit EditWidget(QWidget * parent = nullptr);
};

#endif // EDITWIDGET_H
