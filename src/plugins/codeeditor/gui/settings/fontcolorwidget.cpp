// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontcolorwidget.h"
#include "editorsettings.h"
#include "settingsdefine.h"

#include <DFontComboBox>
#include <DSpinBox>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

const char DefaultFontFamily[] { "Noto Mono" };
const int DefaultFontSize { 10 };
const int DefaultFontZoom { 100 };
const int FontMiniZoomSize { 5 };

DWIDGET_USE_NAMESPACE

class FontColorWidgetPrivate
{
public:
    explicit FontColorWidgetPrivate(FontColorWidget *qq);

    void initUI();
    void initConnection();
    QWidget *createItem(const QString &name, QWidget *widget);

    QList<int> pointSizesForSelectedFont() const;
    void updatePointSizes();
    void updateZoomRange(int fontSize);

    FontColorWidget *q;
    DFontComboBox *fontComboBox { nullptr };
    QComboBox *fontSizeComboBox { nullptr };
    DSpinBox *zoomSpinBox { nullptr };
    int fontSize { 0 };
};

FontColorWidgetPrivate::FontColorWidgetPrivate(FontColorWidget *qq)
    : q(qq)
{
}

void FontColorWidgetPrivate::initUI()
{
    QLabel *infoLabel = new QLabel(FontColorWidget::tr("Font"), q);
    fontComboBox = new DFontComboBox(q);
    fontComboBox->setFontFilters(QFontComboBox::MonospacedFonts);

    fontSizeComboBox = new QComboBox(q);
    fontSizeComboBox->setMinimumWidth(100);
    fontSizeComboBox->setEditable(true);
    auto sizeValidator = new QIntValidator(fontSizeComboBox);
    sizeValidator->setBottom(0);
    fontSizeComboBox->setValidator(sizeValidator);

    zoomSpinBox = new DSpinBox(q);
    zoomSpinBox->setSuffix("%");
    zoomSpinBox->setRange(10, 300);
    zoomSpinBox->setSingleStep(10);

    QHBoxLayout *itemLayout = new QHBoxLayout;
    itemLayout->setSpacing(15);
    itemLayout->addWidget(createItem(FontColorWidget::tr("Family:"), fontComboBox));
    itemLayout->addWidget(createItem(FontColorWidget::tr("Size:"), fontSizeComboBox));
    itemLayout->addWidget(createItem(FontColorWidget::tr("Zoom:"), zoomSpinBox));
    itemLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(infoLabel);
    mainLayout->addLayout(itemLayout);
}

void FontColorWidgetPrivate::initConnection()
{
    q->connect(fontComboBox, &DFontComboBox::currentFontChanged, q, &FontColorWidget::fontSelected);
    q->connect(fontSizeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), q, &FontColorWidget::fontSizeSelected);
}

QWidget *FontColorWidgetPrivate::createItem(const QString &name, QWidget *widget)
{
    QWidget *box = new QWidget(q);
    QHBoxLayout *layout = new QHBoxLayout(box);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(name, q);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(label);
    layout->addWidget(widget);
    return box;
}

QList<int> FontColorWidgetPrivate::pointSizesForSelectedFont() const
{
    QFontDatabase db;
    const QString familyName = fontComboBox->currentFont().family();
    QList<int> sizeLst = db.pointSizes(familyName);
    if (!sizeLst.isEmpty())
        return sizeLst;

    QStringList styles = db.styles(familyName);
    if (!styles.isEmpty())
        sizeLst = db.pointSizes(familyName, styles.first());
    if (sizeLst.isEmpty())
        sizeLst = QFontDatabase::standardSizes();

    return sizeLst;
}

void FontColorWidgetPrivate::updatePointSizes()
{
    const int oldSize = fontSize;
    fontSizeComboBox->clear();
    const QList<int> sizeLst = pointSizesForSelectedFont();
    int idx = -1;
    int i = 0;
    for (; i < sizeLst.count(); ++i) {
        if (idx == -1 && sizeLst.at(i) >= oldSize)
            idx = i;
        fontSizeComboBox->addItem(QString::number(sizeLst.at(i)));
    }
    if (idx != -1)
        fontSizeComboBox->setCurrentIndex(idx);
}

void FontColorWidgetPrivate::updateZoomRange(int fontSize)
{
    auto ratio =  (double)FontMiniZoomSize / fontSize;
    int zoomMiniValue = qRound(ratio * 10) * 10;
    
    zoomSpinBox->setRange(zoomMiniValue, 300);
}

FontColorWidget::FontColorWidget(QWidget *parent)
    : PageWidget(parent),
      d(new FontColorWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

FontColorWidget::~FontColorWidget()
{
    delete d;
}

void FontColorWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    Q_UNUSED(map)

    auto fontFamily = EditorSettings::instance()->value(Node::FontColor, Group::FontGroup, Key::FontFamily, DefaultFontFamily).toString();
    d->fontSize = EditorSettings::instance()->value(Node::FontColor, Group::FontGroup, Key::FontSize, DefaultFontSize).toInt();
    auto fontZoom = EditorSettings::instance()->value(Node::FontColor, Group::FontGroup, Key::FontZoom, DefaultFontZoom).toInt();

    d->fontSizeComboBox->setCurrentText(QString::number(d->fontSize));
    d->fontComboBox->setCurrentText(fontFamily);
    d->zoomSpinBox->setValue(fontZoom);
    d->updatePointSizes();
}

void FontColorWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    QVariantMap fontMap;
    fontMap.insert(Key::FontFamily, d->fontComboBox->currentFont().family());
    fontMap.insert(Key::FontSize, d->fontSizeComboBox->currentText().toInt());
    fontMap.insert(Key::FontZoom, d->zoomSpinBox->value());

    map.insert(Group::FontGroup, fontMap);

    EditorSettings::instance()->setValue(Node::FontColor, Group::FontGroup, Key::FontFamily,
                                         d->fontComboBox->currentFont().family());
    EditorSettings::instance()->setValue(Node::FontColor, Group::FontGroup, Key::FontSize, d->fontSize);
    EditorSettings::instance()->setValue(Node::FontColor, Group::FontGroup, Key::FontZoom, d->zoomSpinBox->value());
}

void FontColorWidget::fontSelected(const QFont &font)
{
    d->updatePointSizes();
}

void FontColorWidget::fontSizeSelected(int index)
{
    const auto sizeStr = d->fontSizeComboBox->itemText(index);
    bool ok = true;
    const int size = sizeStr.toInt(&ok);
    if (ok) {
        d->fontSize = size;
        d->updateZoomRange(size);
    }
}
