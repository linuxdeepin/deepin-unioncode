// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "variablechooser.h"
#include "util/qtcassert.h"
#include "util/macroexpander.h"

#include <DIconButton>
#include <DSearchEdit>
#include <DGuiApplicationHelper>

#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QStyledItemDelegate>
#include <QApplication>

DWIDGET_USE_NAMESPACE

const char VariableFlag[] = "VariableFlag";

enum {
    UnexpandedTextRole = Qt::UserRole,
    CurrentValueDisplayRole
};

class VariableItemDelegate : public QStyledItemDelegate
{
public:
    explicit VariableItemDelegate(QObject *parent = nullptr);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

VariableItemDelegate::VariableItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void VariableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem iOption = option;

    if (!option.state.testFlag(QStyle::State_HasFocus)) {
        QColor color;
        DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
            ? color = Qt::black
            : color = QColor("#c5c8c9");
        iOption.palette.setColor(QPalette::Text, color);
    }

    QStyledItemDelegate::paint(painter, iOption, index);
}

class VariableSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit VariableSortFilterProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent) {}
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        const QModelIndex index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
        if (!index.isValid())
            return false;

        const QRegExp regexp = filterRegExp();
        if (regexp.pattern().isEmpty() || sourceModel()->rowCount(index) > 0)
            return true;

        const QString displayText = index.data(Qt::DisplayRole).toString();
        return displayText.contains(regexp);
    }
};

class VariableChooserPrivate : public QObject
{
    Q_OBJECT
public:
    VariableChooserPrivate(VariableChooser *qq);

    void initUI();
    void initConnect();

    void show();
    void updateCurrentEditor(QWidget *old, QWidget *widget);
    void updateButtonGeometry();
    void updateDescription(const QModelIndex &index, const QModelIndex &);
    void updateFilter(const QString &filter);
    void handleItemActivated(const QModelIndex &index);
    void insertText(const QString &text);

    int buttonMargin() const;
    QWidget *currentWidget() const;

public:
    VariableChooser *q;

    QTreeView *treeView { nullptr };
    QStandardItemModel model;
    VariableSortFilterProxyModel *sortModel { nullptr };
    DSearchEdit *searchEdit { nullptr };
    DIconButton *iconButton { nullptr };
    QLabel *variableDescription { nullptr };

    QLineEdit *lineEdit { nullptr };
};

VariableChooserPrivate::VariableChooserPrivate(VariableChooser *qq)
    : q(qq)
{
}

void VariableChooserPrivate::initUI()
{
    q->setWindowTitle(VariableChooser::tr("Variables"));
    q->setIcon(QIcon::fromTheme("ide"));
    q->setFixedSize(400, 500);

    QWidget *widget = new QWidget(q);
    q->addContent(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    treeView = new QTreeView(q);
    treeView->setIndentation(treeView->indentation() * 7 / 10);
    treeView->setHeaderHidden(true);
    treeView->setEditTriggers(QTreeView::NoEditTriggers);
    treeView->setItemDelegate(new VariableItemDelegate(treeView));

    sortModel = new VariableSortFilterProxyModel(this);
    sortModel->setSourceModel(&model);
    sortModel->sort(0);
    sortModel->setFilterKeyColumn(0);
    sortModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    treeView->setModel(sortModel);

    searchEdit = new DSearchEdit(q);
    iconButton = new DIconButton(q);
    iconButton->setFlat(true);
    iconButton->setIconSize({ 24, 24 });
    iconButton->setIcon(QIcon::fromTheme("binarytools_default"));
    iconButton->setToolTip(VariableChooser::tr("Insert Variable"));
    iconButton->hide();
    iconButton->setCursor(Qt::ArrowCursor);

    variableDescription = new QLabel(q);
    variableDescription->setWordWrap(true);
    variableDescription->setTextInteractionFlags(Qt::TextBrowserInteraction);

    mainLayout->addWidget(searchEdit);
    mainLayout->addWidget(treeView);
    mainLayout->addWidget(variableDescription);
    updateCurrentEditor(nullptr, QApplication::focusWidget());
}

void VariableChooserPrivate::initConnect()
{
    connect(iconButton, &DIconButton::clicked, this, &VariableChooserPrivate::show);
    connect(qobject_cast<QApplication *>(qApp), &QApplication::focusChanged,
            this, &VariableChooserPrivate::updateCurrentEditor);
    connect(searchEdit, &DSearchEdit::textChanged, this, &VariableChooserPrivate::updateFilter);
    connect(treeView, &QTreeView::activated, this, &VariableChooserPrivate::handleItemActivated);
    connect(treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &VariableChooserPrivate::updateDescription);
}

void VariableChooserPrivate::show()
{
    q->show();
    treeView->expandAll();
}

void VariableChooserPrivate::updateCurrentEditor(QWidget *old, QWidget *widget)
{
    Q_UNUSED(old)

    if (!widget)
        return;

    bool handle = false;
    QWidget *parent = widget;
    while (parent) {
        if (parent == q)
            return;
        if (parent == q->parentWidget()) {
            handle = true;
            break;
        }
        parent = parent->parentWidget();
    }
    if (!handle)
        return;

    QLineEdit *previousLineEdit = lineEdit;
    QWidget *previousWidget = currentWidget();
    lineEdit = nullptr;
    auto chooser = widget->property(VariableFlag).value<QWidget *>();
    bool supportsVariables = chooser == q;
    if (auto edit = qobject_cast<QLineEdit *>(widget))
        lineEdit = (supportsVariables && !edit->isReadOnly() ? edit : nullptr);

    QWidget *current = currentWidget();
    if (current != previousWidget) {
        if (previousWidget)
            previousWidget->removeEventFilter(q);
        if (previousLineEdit)
            previousLineEdit->setTextMargins(0, 0, 0, 0);
        if (iconButton) {
            iconButton->hide();
            iconButton->setParent(nullptr);
        }
        if (current) {
            current->installEventFilter(q);   // escape key handling and geometry changes
            int margin = buttonMargin();
            if (lineEdit)
                lineEdit->setTextMargins(0, 0, margin, 0);
            iconButton->setParent(current);
            updateButtonGeometry();
            iconButton->show();
        } else {
            q->hide();
        }
    }
}

void VariableChooserPrivate::updateButtonGeometry()
{
    QWidget *current = currentWidget();
    int margin = buttonMargin();
    iconButton->setGeometry(current->rect().adjusted(
                                                   current->width() - (margin + 4), 8,
                                                   -8, -qMax(0, current->height() - (margin + 4)))
                                    .translated(0, 0));
}

void VariableChooserPrivate::updateDescription(const QModelIndex &index, const QModelIndex &)
{
    variableDescription->setText(model.data(sortModel->mapToSource(index),
                                            CurrentValueDisplayRole)
                                         .toString());
}

void VariableChooserPrivate::updateFilter(const QString &filter)
{
    sortModel->setFilterRegExp(QRegExp::escape(filter));
    treeView->expandAll();
}

void VariableChooserPrivate::handleItemActivated(const QModelIndex &index)
{
    QString text = model.data(sortModel->mapToSource(index), UnexpandedTextRole).toString();
    if (!text.isEmpty())
        insertText(text);
}

void VariableChooserPrivate::insertText(const QString &text)
{
    if (lineEdit) {
        lineEdit->insert(text);
        lineEdit->activateWindow();
    }
}

int VariableChooserPrivate::buttonMargin() const
{
    return 24;
}

QWidget *VariableChooserPrivate::currentWidget() const
{
    return lineEdit ? lineEdit : nullptr;
}

VariableChooser::VariableChooser(QWidget *parent)
    : DDialog(parent),
      d(new VariableChooserPrivate(this))
{
    d->initUI();
    d->initConnect();
    addMacroExpander(globalMacroExpander());
}

VariableChooser::~VariableChooser()
{
    delete d;
}

void VariableChooser::addMacroExpander(MacroExpander *expander)
{
    QStandardItem *group = new QStandardItem(expander->displayName());
    d->model.appendRow(group);

    for (const auto &var : expander->variables()) {
        QStandardItem *item = new QStandardItem(var);
        item->setData("%{" + var + '}', UnexpandedTextRole);
        QString description = expander->description(var);
        const QString value = expander->value(var).toHtmlEscaped();
        if (!value.isEmpty())
            description += QLatin1String("<p>") + tr("Current Value: %1").arg(value);
        item->setData(description, CurrentValueDisplayRole);
        group->appendRow(item);
    }
}

void VariableChooser::addSupportedEdit(QLineEdit *edit, const QByteArray &ownName)
{
    QTC_ASSERT(edit, return );
    edit->setClearButtonEnabled(false);
    edit->setProperty(VariableFlag, QVariant::fromValue<QWidget *>(this));
}

bool VariableChooser::eventFilter(QObject *obj, QEvent *e)
{
    if (obj != d->currentWidget())
        return false;

    if (e->type() == QEvent::Resize || e->type() == QEvent::LayoutRequest) {
        d->updateButtonGeometry();
    } else if (e->type() == QEvent::Hide) {
        close();
    }

    return false;
}
#include "variablechooser.moc"
