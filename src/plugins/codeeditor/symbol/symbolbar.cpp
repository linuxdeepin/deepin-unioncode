// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolbar.h"
#include "symbolview.h"
#include "symbolmanager.h"
#include "gui/texteditor.h"

#include "services/project/projectservice.h"

#include <DStyle>

#include <QHBoxLayout>
#include <QtMath>
#include <QDir>

inline constexpr int IconSize { 12 };

using namespace dpfservice;
DWIDGET_USE_NAMESPACE

CurmbItem::CurmbItem(CurmbType type, int index, QWidget *parent)
    : QWidget(parent)
{
    this->type = type;
    this->index = index;
    setCursor(Qt::PointingHandCursor);
}

CurmbItem::CurmbType CurmbItem::curmbType() const
{
    return type;
}

void CurmbItem::setText(const QString &text)
{
    displayText = text;
    auto rect = fontMetrics().boundingRect(text);
    int w = rect.width() + qCeil(font().pixelSize() / 10);
    if (!isRoot())
        w += spacing + IconSize;

    setFixedWidth(w);
    setFixedHeight(rect.height());
    update();
}

void CurmbItem::setUserData(const QVariant &data)
{
    this->data = data;
}

QVariant CurmbItem::userData() const
{
    return data;
}

void CurmbItem::setSelected(bool selected)
{
    isSelected = selected;
    update();
}

bool CurmbItem::isRoot() const
{
    return 0 == index;
}

void CurmbItem::mousePressEvent(QMouseEvent *event)
{
    setSelected(true);
    Q_EMIT clicked();
    QWidget::mousePressEvent(event);
}

void CurmbItem::enterEvent(QEvent *event)
{
    isHover = true;
    update();
    QWidget::enterEvent(event);
}

void CurmbItem::leaveEvent(QEvent *event)
{
    isHover = false;
    update();
    QWidget::leaveEvent(event);
}

void CurmbItem::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    p.setPen(Qt::NoPen);

    // draw seperator
    if (!isRoot()) {
        auto rect = QRect(0, 0, IconSize, IconSize);
        rect.moveTop((height() - rect.height()) / 2);
        auto icon = QIcon::fromTheme("edit-forward");
        icon.paint(&p, rect);
    }

    // draw text
    auto palette = this->palette();
    if (isSelected)
        p.setPen(palette.color(QPalette::Highlight));
    else if (isHover)
        p.setPen(palette.color(QPalette::BrightText));
    else
        p.setPen(palette.color(QPalette::Text));

    QRect rect;
    if (isRoot()) {
        rect = this->rect();
    } else {
        int sw = spacing + IconSize;
        rect = QRect(sw, 0, width() - sw, height());
    }

    p.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, displayText);
}

SymbolBar::SymbolBar(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 0, 0);
    layout->setSpacing(4);
}

void SymbolBar::setPath(const QString &path)
{
    clear();

    QString displayPath = path;
    auto prjSrv = dpfGetService(ProjectService);
    const auto &allPrj = prjSrv->getAllProjectInfo();

    QString workspaceDir;
    for (const auto &prj : allPrj) {
        const auto &dir = prj.workspaceFolder();
        if (displayPath.startsWith(dir)) {
            displayPath.remove(dir);
            workspaceDir = dir;
            break;
        }
    }

    auto layout = qobject_cast<QHBoxLayout *>(this->layout());
    auto itemList = displayPath.split(QDir::separator(), QString::SkipEmptyParts);
    for (int i = 0; i < itemList.size(); ++i) {
        CurmbItem *item = new CurmbItem(CurmbItem::FilePath, i, this);
        item->setText(itemList[i]);
        item->setToolTip(path);
        QString absolutePath = workspaceDir + QDir::separator() + itemList.mid(0, i).join(QDir::separator());
        item->setUserData(absolutePath);
        layout->addWidget(item, 0, Qt::AlignVCenter | Qt::AlignLeft);
        connect(item, &CurmbItem::clicked, this, &SymbolBar::curmbItemClicked);
    }
    layout->addStretch(1);
}

void SymbolBar::clear()
{
    auto layout = qobject_cast<QHBoxLayout *>(this->layout());
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget())
            delete widget;
        delete item;
    }

    symbolItem = nullptr;
}

void SymbolBar::updateSymbol(int line, int index)
{
    auto editor = qobject_cast<TextEditor *>(sender());
    if (!editor)
        return;

    auto name = SymbolManager::instance()->symbolName(editor->getFile(), line, index);
    if (name.isEmpty())
        return;

    if (!symbolItem) {
        auto layout = qobject_cast<QHBoxLayout *>(this->layout());
        if (layout->count() < 1)
            return;

        symbolItem = new CurmbItem(CurmbItem::Symbol, layout->count() - 1, this);
        layout->insertWidget(layout->count() - 1, symbolItem);
        connect(symbolItem, &CurmbItem::clicked, this, &SymbolBar::curmbItemClicked);
    }

    symbolItem->setUserData(editor->getFile());
    symbolItem->setText(name);
    symbolItem->setToolTip(name);
}

void SymbolBar::curmbItemClicked()
{
    CurmbItem *item = qobject_cast<CurmbItem *>(sender());
    if (!item)
        return;

    if (!symbolView) {
        symbolView = new SymbolView(this);
        connect(symbolView, &SymbolView::hidden, this, [item] { item->setSelected(false); });
    }

    auto rect = item->geometry();
    auto pos = mapToGlobal(rect.bottomLeft());

    switch (item->curmbType()) {
    case CurmbItem::FilePath: {
        const auto &path = item->userData().toString();
        symbolView->setRootPath(path);
        symbolView->show(pos);
    } break;
    case CurmbItem::Symbol: {
        const auto &path = item->userData().toString();
        if (symbolView->setSymbolPath(path))
            symbolView->show(pos);
        else
            item->setSelected(false);
    } break;
    }
}
