#include "FileWidget.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

FileWidget::FileWidget(const QIcon &icon, const QString &text, QWidget *parent)
   : QFrame(parent)
   , mIcon(icon)
   , mButton(new QPushButton(mIcon, ""))
   , mText(new QLabel(text))
{
   const auto itemLayout = new QHBoxLayout(this);
   itemLayout->setContentsMargins(QMargins());
   mButton->setStyleSheet("max-width: 15px; min-width: 15px; max-height: 15px; min-height: 15px;");

   itemLayout->addWidget(mButton);
   itemLayout->addWidget(mText);

   if (!icon.isNull())
      connect(mButton, &QPushButton::clicked, this, [this]() { emit clicked(); });
}

FileWidget::FileWidget(const QString &icon, const QString &text, QWidget *parent)
   : FileWidget(QIcon(icon), text, parent)
{
}

QString FileWidget::text() const
{
   return mText->text();
}

void FileWidget::setText(const QString &text)
{
   mText->setText(text);
}

QSize FileWidget::sizeHint() const
{
   auto size = QFrame::sizeHint();
   size.setWidth(size.width() + layout()->spacing());

   return size;
}

void FileWidget::setTextColor(const QColor &color)
{
   mColor = color;
   mText->setStyleSheet(QString("color: %1").arg(mColor.name()));
}
