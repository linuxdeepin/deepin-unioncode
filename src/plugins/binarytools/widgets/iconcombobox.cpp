// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconcombobox.h"

#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DPaletteHelper
#    include <DPaletteHelper>
#endif

#include <QHBoxLayout>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QStylePainter>
#include <QBitmap>

DGUI_USE_NAMESPACE
#ifdef DTKWIDGET_CLASS_DPaletteHelper
DWIDGET_USE_NAMESPACE
#endif

class IconItemDelegate : public QStyledItemDelegate
{
public:
    explicit IconItemDelegate(QListView *parent);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QRectF itemIconRect(const QRectF &itemRect) const;
    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    void paintItemBackgroundAndGeomerty(QPainter *painter, const QStyleOptionViewItem &option) const;
    void paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;

    QListView *view;
};

IconItemDelegate::IconItemDelegate(QListView *parent)
    : QStyledItemDelegate(parent),
      view(parent)
{
}

void IconItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    paintItemBackgroundAndGeomerty(painter, opt);
    paintItemIcon(painter, opt, index);
}

QSize IconItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return { 36, 36 };
}

QRectF IconItemDelegate::itemIconRect(const QRectF &itemRect) const
{
    QRectF iconRect = itemRect;
    iconRect.setSize(view->iconSize());
    iconRect.moveLeft(itemRect.left() + (itemRect.width() - iconRect.width()) / 2.0);
    iconRect.moveTop(iconRect.top() + 8);
    return iconRect;
}

QPixmap IconItemDelegate::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull())
        return QPixmap();

    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    auto px = icon.pixmap(size, mode, state);
    px.setDevicePixelRatio(qApp->devicePixelRatio());

    return px;
}

void IconItemDelegate::paintItemBackgroundAndGeomerty(QPainter *painter,
                                                      const QStyleOptionViewItem &option) const
{
    painter->save();

    bool isSelected = (option.state & QStyle::State_Selected);

#ifdef DTKWIDGET_CLASS_DPaletteHelper
    const DPalette &pl = DPaletteHelper::instance()->palette(option.widget);
#else
    const DPalette &pl = DGuiApplicationHelper::instance()->applicationPalette();
#endif

    QColor backgroundColor = pl.color(QPalette::Active, QPalette::Shadow);
    bool isHover = option.state & QStyle::StateFlag::State_MouseOver;
    if (isSelected) {
        backgroundColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::LightLively);
    } else if (isHover) {
        backgroundColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ObviousBackground);
    }

    QRectF backgroundRect = option.rect;
    QSizeF iconSize = view->iconSize();
    // 左右上下和icon的边距都是8
    backgroundRect.setSize(iconSize + QSizeF(2 * 8, 2 * 8));
    // for checkmark
    qreal backgroundx = (option.rect.width() - backgroundRect.width()) / 2.0;
    backgroundRect.moveLeft(backgroundRect.left() + backgroundx);
    // draw background
    QPainterPath path;
    path.addRoundedRect(backgroundRect, 8, 8);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillPath(path, backgroundColor);
    painter->restore();
}

void IconItemDelegate::paintItemIcon(QPainter *painter,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    // init icon geomerty
    QRectF iconRect = itemIconRect(option.rect);

    // draw icon
    QIcon::Mode iconMode = QIcon::Normal;
    if (!(option.state.testFlag(QStyle::State_Enabled)))
        iconMode = QIcon::Disabled;
    if (option.state.testFlag(QStyle::State_Selected))
        iconMode = QIcon::Selected;
    const qreal pixelRatio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(option.icon, iconRect.size().toSize(), pixelRatio,
                                      iconMode);
    qreal x = iconRect.x();
    qreal y = iconRect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    y += (iconRect.size().height() - h) / 2.0;
    x += (iconRect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
}

IconComboBox::IconComboBox(QWidget *parent)
    : QComboBox(parent)
{
    initUI();
    initData();
    initConnections();
}

void IconComboBox::initUI()
{
    iconFrame = new QFrame(this, Qt::Popup);
    iconFrame->setAttribute(Qt::WA_TranslucentBackground);
    iconFrame->installEventFilter(this);
    QHBoxLayout *layout = new QHBoxLayout(iconFrame);
    layout->setContentsMargins(0, 0, 0, 0);

    iconView = new QListView(this);
    iconView->setFrameShape(QFrame::NoFrame);
    iconView->setViewMode(QListView::IconMode);
    iconView->setFlow(QListView::LeftToRight);
    iconView->setDragDropMode(QListView::NoDragDrop);
    iconView->setEditTriggers(QListView::NoEditTriggers);
    iconView->setSelectionMode(QListView::SingleSelection);
    iconView->setSizeAdjustPolicy(QListView::AdjustToContents);
    iconView->setSpacing(9);
    iconView->setIconSize({ 20, 20 });
    iconView->setModel(&iconModel);
    iconView->setItemDelegate(new IconItemDelegate(iconView));

    layout->addWidget(iconView);
}

void IconComboBox::initData()
{
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_default"), "binarytools_default"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_file"), "binarytools_file"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_flag"), "binarytools_flag"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_light"), "binarytools_light"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_packing"), "binarytools_packing"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_combo"), "binarytools_combo"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_signature"), "binarytools_signature"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_translate"), "binarytools_translate"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_star"), "binarytools_star"));
    iconModel.appendRow(new QStandardItem(QIcon::fromTheme("binarytools_command"), "binarytools_command"));
}

void IconComboBox::initConnections()
{
    connect(iconView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection &selected, const QItemSelection &deselected) {
                QModelIndexList selectedIndexes = selected.indexes();
                if (!selectedIndexes.isEmpty()) {
                    auto icon = iconModel.data(selectedIndexes.first()).toString();
                    if (!icon.isEmpty())
                        iconName = icon;
                }

                if (iconFrame->isVisible())
                    iconFrame->hide();
            });
}

void IconComboBox::showPopup()
{
    QStyle *const style = this->style();
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    QRect rect(style->subControlRect(QStyle::CC_ComboBox, &opt,
                                     QStyle::SC_ComboBoxListBoxPopup, this));
    QPoint below = mapToGlobal(rect.bottomLeft());
    below.setY(below.y() + 4);

    iconFrame->setFixedWidth(width());
    iconFrame->show();
    iconFrame->move(below);
}

void IconComboBox::setIcon(const QString &icon)
{
    iconName = icon;
    iconView->selectionModel()->clearSelection();
    QList<QStandardItem *> items = iconModel.findItems(icon);
    if (!items.isEmpty()) {
        QModelIndex index = iconModel.indexFromItem(items.first());
        iconView->setCurrentIndex(index);
    } else {
        auto index = iconModel.index(0, 0);
        iconView->setCurrentIndex(index);
        iconName = index.data().toString();
    }

    iconView->setFlow(QListView::LeftToRight);
    update();
}

QString IconComboBox::icon()
{
    if (iconName.isEmpty())
        iconName = "binarytools_default";

    return iconName;
}

void IconComboBox::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    QComboBox::initStyleOption(&opt);
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);
    opt.currentIcon = QIcon::fromTheme(icon());
    // draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

bool IconComboBox::eventFilter(QObject *obj, QEvent *e)
{
    if (iconFrame) {
        switch (e->type()) {
        case QEvent::Paint: {
            QPainter painter(iconFrame);
            painter.setRenderHint(QPainter::Antialiasing);

            auto p = iconFrame->palette();
            painter.setPen(Qt::NoPen);
            painter.setBrush(p.brush(QPalette::Normal, QPalette::Base));

            QPainterPath path;
            path.addRoundedRect(iconFrame->rect(), 18, 18);
            painter.drawPath(path);
        } break;
        case QEvent::Resize: {
            QPixmap pixmap(iconFrame->size());
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(Qt::black);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(iconFrame->rect(), 18, 18);
            iconFrame->setMask(pixmap.mask());
        } break;
        default:
            break;
        }
    }

    return QComboBox::eventFilter(obj, e);
}
