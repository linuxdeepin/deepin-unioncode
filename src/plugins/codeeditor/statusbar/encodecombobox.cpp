// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encodecombobox.h"
#include "utils/editorutils.h"

#include <DStyle>
#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#include <DPaletteHelper>
#else
#include <DApplicationHelper>
#endif

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

EncodeComboBox::EncodeComboBox(QWidget *parent)
    : QFrame(parent)
{
    initUI();
    initMenuData();
    initConnection();
}

void EncodeComboBox::initUI()
{
    toolBtn = new DToolButton(this);
    toolBtn->setFocusPolicy(Qt::NoFocus);
    toolBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBtn->setArrowType(Qt::NoArrow);
    toolBtn->setFixedHeight(28);
    toolBtn->installEventFilter(this);

    menu = new QMenu(this);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(toolBtn);
}

void EncodeComboBox::initConnection()
{
    connect(menu, &QMenu::triggered, this, [this](QAction *action) {
        if (curEncodeName != action->text())
            Q_EMIT encodeChanged(action->text());
    });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this] { toolBtn->setIcon(createIcon()); });
}

void EncodeComboBox::initMenuData()
{
    static QMap<QString, QString> translationMap {
        { "Unicode", tr("Unicode") },
        { "WesternEuropean", tr("WesternEuropean") },
        { "CentralEuropean", tr("CentralEuropean") },
        { "Baltic", tr("Baltic") },
        { "Cyrillic", tr("Cyrillic") },
        { "Arabic", tr("Arabic") },
        { "Greek", tr("Greek") },
        { "Hebrew", tr("Hebrew") },
        { "Turkish", tr("Turkish") },
        { "Thai", tr("Thai") },
        { "Celtic", tr("Celtic") },
        { "SouthEasternEuropean", tr("SouthEasternEuropean") },
        { "ChineseSimplified", tr("ChineseSimplified") },
        { "ChineseTraditional", tr("ChineseTraditional") },
        { "Japanese", tr("Japanese") },
        { "Korean", tr("Korean") },
        { "Vietnamese", tr("Vietnamese") }
    };

    auto groupEncodeVec = EditorUtils::supportEncoding();
    if (!groupEncodeVec.isEmpty()) {
        int cnt = groupEncodeVec.size();
        for (int i = 0; i < cnt; i++) {
            const auto &groupName = groupEncodeVec[i].first;
            QMenu *groupMenu = new QMenu(translationMap.value(groupName, groupName));
            foreach (const QString &var, groupEncodeVec[i].second) {
                groupMenu->addAction(var);
            }
            menu->addMenu(groupMenu);
        }
    }
}

void EncodeComboBox::setEncodeName(const QString &encoding)
{
    if (curEncodeName == encoding)
        return;

    curEncodeName = encoding;
    toolBtn->setIcon(createIcon());

    for (auto ac : menu->actions()) {
        setCheckedExclusive(ac, encoding);
    }
}

QString EncodeComboBox::encodeName() const
{
    return curEncodeName;
}

bool EncodeComboBox::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == toolBtn) {
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            if (mouseEvent->button() == Qt::LeftButton) {
                isPressed = true;
                toolBtn->setIcon(createIcon());
                return true;
            } else if (mouseEvent->button() == Qt::RightButton)
                return true;
        } else if (e->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            if (mouseEvent->button() == Qt::LeftButton) {
                isPressed = false;
                toolBtn->setIcon(createIcon());
                showContextMenu();
            }
        }
    }

    return QFrame::eventFilter(obj, e);
}

QIcon EncodeComboBox::createIcon()
{
#ifdef DTKWIDGET_CLASS_DPaletteHelper
    DPalette dpalette = DPaletteHelper::instance()->palette(toolBtn);
#else
    DPalette dpalette = DApplicationHelper::instance()->palette(toolBtn);
#endif
    QColor textColor;
    QPixmap arrowPixmap = QCommonStyle().standardPixmap(QStyle::SP_ArrowDown);
    if (isPressed) {
        textColor = dpalette.color(DPalette::Highlight);
        QPainter arrowPainter(&arrowPixmap);
        arrowPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        arrowPainter.fillRect(arrowPixmap.rect(), dpalette.color(DPalette::Highlight));
        arrowPainter.end();
    } else {
        textColor = dpalette.color(DPalette::WindowText);
    }

    QFontMetrics metrics(font());
    int fontWidth = metrics.width(curEncodeName) + 20;
    int fontHeight = metrics.size(Qt::TextSingleLine, curEncodeName).height();
    int iconW = 8;
    int iconH = 5;

    int totalWidth = fontWidth + iconW + 10;
    toolBtn->setFixedSize(totalWidth, 28);
    toolBtn->setIconSize(QSize(totalWidth, 28));

    qreal rate = this->devicePixelRatioF();
    QPixmap icon(QSize(totalWidth, 28) * rate);
    icon.setDevicePixelRatio(rate);
    icon.fill(Qt::transparent);

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);

    painter.save();
    painter.setFont(font());
    painter.setPen(textColor);
    painter.drawText(QRectF(10, (28 - fontHeight) / 2, fontWidth, fontHeight), curEncodeName);
    painter.restore();
    painter.drawPixmap(QRectF(fontWidth, (28 - iconH) / 2, iconW, iconH), arrowPixmap, arrowPixmap.rect());

    painter.end();
    return icon;
}

void EncodeComboBox::showContextMenu()
{
    QPoint center = this->mapToGlobal(this->rect().center());
    int menuHeight = menu->sizeHint().height();
    int menuWidth = menu->sizeHint().width();
    center.setY(center.y() - menuHeight - this->rect().height() / 2);
    center.setX(center.x() - menuWidth / 2);
    menu->move(center);
    menu->exec();

    QEvent event(QEvent::HoverLeave);
    QApplication::sendEvent(toolBtn, &event);
}

void EncodeComboBox::setCheckedExclusive(QAction *action, const QString &name)
{
    if (!action)
        return;

    if (action->menu()) {
        for (auto ac : action->menu()->actions()) {
            setCheckedExclusive(ac, name);
        }
    } else {
        if (action->text() != name) {
            action->setCheckable(false);
            action->setChecked(false);
        } else {
            action->setCheckable(true);
            action->setChecked(true);
        }
    }
}
