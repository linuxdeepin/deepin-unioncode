#include "refactorwidget.h"
#include "codelens.h"
#include <QGridLayout>

class RefactorWidgetPrivate
{
    friend class RefactorWidget;
    CodeLens *lens {nullptr};
    QGridLayout *gLayout {nullptr};
};

RefactorWidget *RefactorWidget::instance()
{
    static RefactorWidget ins;
    return &ins;
}

RefactorWidget::RefactorWidget(QWidget *parent)
    : QWidget(parent)
    , d (new RefactorWidgetPrivate())
{
    d->lens = new CodeLens();
    d->gLayout = new QGridLayout();
    d->gLayout->addWidget(d->lens);
    setLayout(d->gLayout);
    QObject::connect(d->lens, &CodeLens::doubleClicked, this, &RefactorWidget::doubleClicked);
}

RefactorWidget::~RefactorWidget()
{
    if (d) {
        delete d;
    }
}

void RefactorWidget::displayReference(const lsp::References &data)
{
    d->lens->setData(data);
}
