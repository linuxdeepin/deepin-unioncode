// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolwidget.h"
#include "symbolview.h"
#include "symbolmanager.h"
#include "gui/texteditor.h"

#include "base/abstractwidget.h"
#include "services/window/windowservice.h"

#include <DLabel>
#include <DToolButton>

#include <QVBoxLayout>

inline constexpr char SymbolDockName[] { "OUTLINE" };

using namespace dpfservice;
DWIDGET_USE_NAMESPACE

class SymbolWidgetPrivate : public QObject
{
public:
    explicit SymbolWidgetPrivate(SymbolWidget *qq);

    void initUI();
    void initConnection();

    DToolButton *registerOperator(const QIcon &icon, const QString &description, std::function<void()> handler);

    void handleFollowCursor();
    void handleEditorCursorChanged(int line, int col);
    void handleClear();
    void handleUpdateSymbols();

public:
    SymbolWidget *q;

    DLabel *msgLabel { nullptr };
    SymbolView *symbolView { nullptr };
    TextEditor *editor { nullptr };

    DToolButton *followCursorBtn { nullptr };

    WindowService *winSrv { nullptr };
};

SymbolWidgetPrivate::SymbolWidgetPrivate(SymbolWidget *qq)
    : q(qq)
{
}

void SymbolWidgetPrivate::initUI()
{
    q->setAutoFillBackground(true);
    q->setBackgroundRole(QPalette::Base);
    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);

    msgLabel = new DLabel(q);
    msgLabel->setForegroundRole(DPalette::PlaceholderText);
    msgLabel->setWordWrap(true);

    symbolView = new SymbolView(SymbolView::DoubleClick, q);
    symbolView->setFrameShape(QFrame::NoFrame);

    layout->addWidget(msgLabel, 0, Qt::AlignTop);
    layout->addWidget(symbolView, 1);
}

void SymbolWidgetPrivate::initConnection()
{
    connect(SymbolManager::instance(), &SymbolManager::symbolChanged, this,
            [this](const QString &file) {
                if (!editor || editor->getFile() != file)
                    return;
                handleUpdateSymbols();
            });
}

DToolButton *SymbolWidgetPrivate::registerOperator(const QIcon &icon, const QString &description, std::function<void()> handler)
{
    if (!winSrv)
        winSrv = dpfGetService(WindowService);

    DToolButton *btn = new DToolButton(q);
    btn->setIcon(icon);
    btn->setIconSize({ 16, 16 });
    btn->setToolTip(description);
    connect(btn, &DToolButton::clicked, this, handler);

    winSrv->registerWidgetToDockHeader(SymbolDockName, btn);
    return btn;
}

void SymbolWidgetPrivate::handleFollowCursor()
{
    if (followCursorBtn->isChecked() && editor) {
        int line = 0, col = 0;
        editor->lineIndexFromPosition(editor->cursorPosition(), &line, &col);
        handleEditorCursorChanged(line, col);
        connect(editor, &TextEditor::cursorPositionChanged, this, &SymbolWidgetPrivate::handleEditorCursorChanged, Qt::UniqueConnection);
    } else {
        disconnect(editor, &TextEditor::cursorPositionChanged, this, nullptr);
    }
}

void SymbolWidgetPrivate::handleEditorCursorChanged(int line, int col)
{
    const auto &info = SymbolManager::instance()->findSymbol(editor->getFile(), line, col);
    symbolView->selectSymbol(info.symbolName, line, col);
}

void SymbolWidgetPrivate::handleClear()
{
    disconnect(editor, nullptr, q, nullptr);
    disconnect(editor, nullptr, this, nullptr);
    editor = nullptr;
    msgLabel->clear();
    msgLabel->setVisible(false);
    symbolView->setVisible(false);
}

void SymbolWidgetPrivate::handleUpdateSymbols()
{
    if (!editor)
        return;

    symbolView->setSymbolPath(editor->getFile());
    if (symbolView->isEmpty()) {
        msgLabel->setVisible(true);
        const auto &fileName = QFileInfo(editor->getFile()).fileName();
        msgLabel->setText(SymbolWidget::tr("No symbols found in document \"%1\"").arg(fileName));
        symbolView->setVisible(false);
    } else {
        msgLabel->setVisible(false);
        symbolView->setVisible(true);
        handleFollowCursor();
    }
}

SymbolWidget::SymbolWidget(QWidget *parent)
    : QWidget(parent),
      d(new SymbolWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

SymbolWidget::~SymbolWidget()
{
    delete d;
}

void SymbolWidget::setEditor(TextEditor *editor)
{
    if (d->editor == editor)
        return;

    if (d->editor)
        d->handleClear();

    d->editor = editor;
    if (d->editor) {
        d->handleUpdateSymbols();
        d->handleFollowCursor();
        connect(d->editor, &TextEditor::destroyed, d, &SymbolWidgetPrivate::handleClear);
    }
}

void SymbolWidget::registerDockHeader()
{
    d->followCursorBtn = d->registerOperator(QIcon::fromTheme("focus_auto"),
                                             tr("Follow Cursor"),
                                             std::bind(&SymbolWidgetPrivate::handleFollowCursor, d));
    d->followCursorBtn->setCheckable(true);

    d->registerOperator(QIcon::fromTheme("expand_all"),
                        tr("Expand All"),
                        std::bind(&SymbolView::expandAll, d->symbolView));
    d->registerOperator(QIcon::fromTheme("collapse_all"),
                        tr("Collapse All"),
                        std::bind(&SymbolView::collapseAll, d->symbolView));
}

void SymbolWidget::showEvent(QShowEvent *event)
{
    static std::once_flag flg;
    std::call_once(flg, [this] {
        QTimer::singleShot(10, this, [this] {
            d->winSrv->splitWidgetOrientation(WN_WORKSPACE, SymbolDockName, Qt::Vertical);
        });
    });

    QWidget::showEvent(event);
}

SymbolWidgetGenerator *SymbolWidgetGenerator::instance()
{
    static SymbolWidgetGenerator ins;
    return &ins;
}

void SymbolWidgetGenerator::registerToDock(QWidget *parent)
{
    static std::once_flag flg;
    std::call_once(flg, [&] {
        WindowService *winSrv = dpfGetService(WindowService);
        symbWidget = new SymbolWidget(parent);

        auto dockWidget = new AbstractWidget(symbWidget);
        dockWidget->setDisplayIcon(QIcon::fromTheme("symbol_tree"));
        winSrv->registerWidgetToMode(SymbolDockName, dockWidget, CM_EDIT, Position::Left, false, true);
        winSrv->setDockHeaderName(SymbolDockName, tr("OUTLINE"));
        symbWidget->registerDockHeader();
    });
}

SymbolWidget *SymbolWidgetGenerator::symbolWidget()
{
    return symbWidget;
}
