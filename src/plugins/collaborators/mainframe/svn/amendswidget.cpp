#include "amendswidget.h"
#include "CommitInfo.h"
#include "filemodifywidget.h"
#include "CommitInfoPanel.h"

#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

const QString Description = QTextEdit::tr("Description");
const QString Summary = QLineEdit::tr("Summary");
const QString Commit = QPushButton::tr("Commit");
const QString RevertAll = QPushButton::tr("Revert All");

AmendsWidget::AmendsWidget(QWidget *parent, Qt::WindowFlags f)
    : QFrame (parent, f)
    , fListAdded(new FileModifyWidget)
    , fListSource(new FileModifyWidget)
    , vLayoutAmend(new QVBoxLayout)
    , hLayPbt(new QHBoxLayout)
    , pbtCommit(new QPushButton(Commit))
    , pbtRevertAll(new QPushButton(RevertAll))
    , vLayoutMain(new QVBoxLayout)
{
    pbtRevertAll->setObjectName("warningButton");
    pbtCommit->setObjectName("applyActionBtn");
    QObject::connect(pbtRevertAll, &QPushButton::clicked,
                     this, &AmendsWidget::revertAllClicked);
    QObject::connect(pbtCommit, &QPushButton::clicked,
                     this, &AmendsWidget::commitClicked);
    vLayoutMain->addWidget(fListSource);
    vLayoutMain->addLayout(vLayoutAmend);
    vLayoutMain->addWidget(fListAdded);
    hLayPbt->addWidget(pbtRevertAll);
    hLayPbt->addWidget(pbtCommit);
    vLayoutMain->addLayout(hLayPbt);
    setLayout(vLayoutMain);
}

AmendsWidget::~AmendsWidget()
{
}

void AmendsWidget::reflashAmends(const QSet<QString> &keys)
{
    for (int i = 0; i < vLayoutAmend->layout()->count(); ++i) {
        delete vLayoutAmend->layout()->itemAt(i);
    }

    QSet<QString> labelKey = keys;
    QVBoxLayout *lineLayout = new QVBoxLayout();
    if (labelKey.contains(Summary)) {
        labelKey.remove(Summary);
        auto lineEdit = new QLineEdit();
        lineEdit->setObjectName("leCommitTitle");
        lineEdit->setPlaceholderText(Summary);
        lineLayout->addWidget(lineEdit);
    }

    if (labelKey.contains(Description)) {
        labelKey.remove(Description);
        auto textEdit = new QTextEdit();
        textEdit->setObjectName("teDescription");
        textEdit->setPlaceholderText(Description);
        lineLayout->addWidget(textEdit);
    }

    auto itera = labelKey.rbegin();
    while(itera != labelKey.rend()) {
        auto lineEdit = new QLineEdit();
        lineEdit->setObjectName("leCommitTitle");
        lineEdit->setPlaceholderText(*itera);
        lineLayout->insertWidget(0, lineEdit);
        itera ++;
    }
    vLayoutAmend->addLayout(lineLayout);
}

QHash<QString, QString> AmendsWidget::amendValues()
{
    QHash<QString, QString> result{};
    auto item = vLayoutAmend->layout()->itemAt(0);
    if (item) {
        auto childLayout = item->layout();
        if (childLayout) {
            for (int i = 0; i < childLayout->layout()->count(); ++i) {
                auto widget = childLayout->layout()->itemAt(i)->widget();
                QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget);
                QTextEdit *textEdit = qobject_cast<QTextEdit*>(widget);
                if (lineEdit) {
                    result[lineEdit->placeholderText()] = lineEdit->text();
                }
                if (textEdit) {
                    result[lineEdit->placeholderText()] = textEdit->toPlainText();
                }
            }
        }
    }
    return result;
}

QString AmendsWidget::amendValue(const QString &key)
{
    QString result{};
    QLayoutItem *item = vLayoutAmend->layout()->itemAt(0);
    if (item) {
        auto childLayout = item->layout();
        if (childLayout) {
            for (int i = 0; i < childLayout->layout()->count(); ++i) {
                auto widget = childLayout->layout()->itemAt(i)->widget();
                QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget);
                QTextEdit *textEdit = qobject_cast<QTextEdit*>(widget);
                if (lineEdit && lineEdit->placeholderText() == key) {
                    result = lineEdit->text();
                }
                if (textEdit && textEdit->placeholderText() == key) {
                    result =textEdit->toPlainText();
                }
            }
        }
    }
    return result;
}

FileModifyWidget *AmendsWidget::fileAddedWidget()
{
    return fListAdded;
}

FileModifyWidget *AmendsWidget::fileSourceWidget()
{
    return fListSource;
}
