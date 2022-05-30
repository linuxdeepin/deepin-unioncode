#include "findtoolwindow.h"
#include "searchresultwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>
#include <QStackedWidget>

#define LABEL_WIDTH (150)

class FindToolWindowPrivate
{
    QStackedWidget *stackedWidget;

    friend class FindToolWindow;
};

FindToolWindow::FindToolWindow(QWidget *parent)
    : QWidget(parent)
    , d(new FindToolWindowPrivate())
{
    setupUi();
}

void FindToolWindow::setupUi()
{
    d->stackedWidget = new QStackedWidget();
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(d->stackedWidget);

    QWidget *searchParamWidget = new QWidget();
    QWidget *searchResultWidget = new QWidget();

    addSearchParamWidget(searchParamWidget);
    addSearchResultWidget(searchResultWidget);

    d->stackedWidget->addWidget(searchParamWidget);
    d->stackedWidget->addWidget(searchResultWidget);

    d->stackedWidget->setCurrentIndex(0);

    setLayout(vLayout);
}

void FindToolWindow::addSearchParamWidget(QWidget *parentWidget)
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    parentWidget->setLayout(hLayout);

    QWidget *widget = new QWidget();
    widget->setFixedSize(725, 300);
    hLayout->addWidget(widget, 0, Qt::AlignLeft);

    QVBoxLayout *vLayout = new QVBoxLayout();
    widget->setLayout(vLayout);

    QHBoxLayout *scopeLayout = new QHBoxLayout();
    QLabel *scopeLabel = new QLabel("Scope:");
    scopeLabel->setAlignment(Qt::AlignRight);
    scopeLabel->setFixedWidth(LABEL_WIDTH);
    QComboBox *scopeComboBox = new QComboBox();
    scopeComboBox->addItem("All Projects");
    scopeComboBox->addItem("Current Project");
    scopeComboBox->addItem("Current File");
    scopeLayout->addWidget(scopeLabel);
    scopeLayout->addWidget(scopeComboBox);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel("Search for:");
    searchLabel->setAlignment(Qt::AlignRight);
    searchLabel->setFixedWidth(LABEL_WIDTH);
    QLineEdit *searchLineEdit = new QLineEdit();
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchLineEdit);

    QHBoxLayout *ruleLayout = new QHBoxLayout();
    QCheckBox *senseCheckBox = new QCheckBox("Case sensitive");
    QCheckBox *wordsCheckBox = new QCheckBox("Whole words only");
    QCheckBox *regularCheckBox = new QCheckBox("Use regular expressions");
    ruleLayout->addStretch();
    ruleLayout->addWidget(senseCheckBox);
    ruleLayout->addWidget(wordsCheckBox);
    ruleLayout->addWidget(regularCheckBox);

    QHBoxLayout *patternLayout = new QHBoxLayout();
    QLabel *patternLabel = new QLabel("File pattern:");
    patternLabel->setAlignment(Qt::AlignRight);
    patternLabel->setFixedWidth(LABEL_WIDTH);
    QLineEdit *patternLineEdit = new QLineEdit();
    patternLayout->addWidget(patternLabel);
    patternLayout->addWidget(patternLineEdit);

    QHBoxLayout *expatternLayout = new QHBoxLayout();
    QLabel *expatternLabel = new QLabel("Exclusion pattern:");
    expatternLabel->setAlignment(Qt::AlignRight);
    expatternLabel->setFixedWidth(LABEL_WIDTH);
    QLineEdit *expatternLineEdit = new QLineEdit();
    expatternLayout->addWidget(expatternLabel);
    expatternLayout->addWidget(expatternLineEdit);

    QPushButton *searchBtn = new QPushButton("Search");
    QPushButton *replaceBtn = new QPushButton("Search && Replace");
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(searchBtn);
    btnLayout->addWidget(replaceBtn);

    connect(searchBtn, &QAbstractButton::clicked, this, &FindToolWindow::search);
    connect(replaceBtn, &QAbstractButton::clicked, this, &FindToolWindow::replace);

    vLayout->addLayout(scopeLayout);
    vLayout->addLayout(searchLayout);
    vLayout->addLayout(ruleLayout);
    vLayout->addLayout(patternLayout);
    vLayout->addLayout(expatternLayout);
    vLayout->addLayout(btnLayout);
}

void FindToolWindow::addSearchResultWidget(QWidget *parentWidget)
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    parentWidget->setLayout(vLayout);

    SearchResultWindow *widget = new SearchResultWindow();
    connect(widget, &SearchResultWindow::back, this, &FindToolWindow::switchSearchParamWidget);
    vLayout->addWidget(widget);
}

void FindToolWindow::search()
{
    d->stackedWidget->setCurrentIndex(1);
}

void FindToolWindow::replace()
{
    d->stackedWidget->setCurrentIndex(1);
}

void FindToolWindow::switchSearchParamWidget()
{
    d->stackedWidget->setCurrentIndex(0);
}
