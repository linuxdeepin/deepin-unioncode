#ifndef SCINTILLAEDITEXTERN_H
#define SCINTILLAEDITEXTERN_H

#include "ScintillaEdit.h"
#include "style/stylelsp.h"
#include "style/stylesci.h"
#include <QString>

class ScintillaEditExternPrivate;
class ScintillaEditExtern : public ScintillaEdit
{
    Q_OBJECT
    ScintillaEditExternPrivate *const d;
public:
    explicit ScintillaEditExtern(QWidget *parent = nullptr);
    virtual ~ScintillaEditExtern();

    virtual QString supportLanguage(){return "";} // 当前编辑器支持的语言类型
    static QString fileLanguage(const QString &path); // 获取注册文件中语言支持

    virtual void setFile(const QString &filePath);
    virtual void setFile(const QString &filePath, const Head &projectHead);

    QString file() const;
    Head projectHead();

    void debugPointAllDelete();
    void jumpToLine(int line);
    void jumpToRange(Scintilla::Position start, Scintilla::Position end);
    void runningToLine(int line);
    void runningEnd();
    void saveText();
    bool isLeave();
    void replaceRange(Scintilla::Position start, Scintilla::Position end, const QString &text);
    QPair<long int, long int> findText(long int start, long int end, const QString &text);
    void findNext(const QString &srcText);
    void replaceAll(const QString &srcText, const QString &destText);

signals:
    void hovered(Scintilla::Position position);
    void hoverCleaned(Scintilla::Position position);
    void definitionHover(Scintilla::Position position);
    void definitionHoverCleaned(Scintilla::Position position);
    void textInserted(Scintilla::Position position,
                      Scintilla::Position length, Scintilla::Position linesAdded,
                      const QByteArray &text, Scintilla::Position line);
    void textDeleted(Scintilla::Position position,
                     Scintilla::Position length, Scintilla::Position linesAdded,
                     const QByteArray &text, Scintilla::Position line);
    void completed(Scintilla::Position position);
    void completeCleaned();
    void indicClicked(Scintilla::Position position);
    void indicReleased(Scintilla::Position position);
    void saved(const QString &file);
    void replaceed(const QString &file, Scintilla::Position start,
                   Scintilla::Position end, const QString &text);
    void selectionMenu(QContextMenuEvent *event);

private slots:
    void sciModified(Scintilla::ModificationFlags type, Scintilla::Position position,
                     Scintilla::Position length, Scintilla::Position linesAdded,
                     const QByteArray &text, Scintilla::Position line,
                     Scintilla::FoldLevel foldNow, Scintilla::FoldLevel foldPrev);
    void sciNotify(Scintilla::NotificationData *data);
    void sciUpdateUi(Scintilla::Update update);
    void sciDwellStart(int x, int y);
    void sciDwellEnd(int x, int y);

protected:
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void sciMarginClicked(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin);
};

#endif // SCINTILLAEDITEXTERN_H
