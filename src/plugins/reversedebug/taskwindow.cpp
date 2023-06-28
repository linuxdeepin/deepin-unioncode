// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskwindow.h"
#include "taskmodel.h"
#include "taskfiltermodel.h"
#include "timelinewidget.h"

#include <QDir>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QListView>
#include <QTextLayout>
#include <QTextLine>
#include <QtDebug>

namespace {
const int ELLIPSIS_GRADIENT_WIDTH = 16;
}

namespace ReverseDebugger {
namespace Internal {

class TaskView : public QListView
{
public:
    TaskView(QWidget *parent = nullptr);
    ~TaskView();
    void resizeEvent(QResizeEvent *e);
    void contextMenuEvent(QContextMenuEvent*);
private:
};

class TaskWidget : public QWidget
{
    // Q_OBJECT
public:
    TaskWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
    }
    ~TaskWidget()
    {
    }

    void setup(TimelineWidget *timeline, TaskView *listview)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(0);
        setLayout(layout);
        layout->addWidget(timeline);
        layout->addWidget(listview);
    }
};

class TaskDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    friend class TaskView; // for using Positions::minimumSize()

public:
    TaskDelegate(QObject * parent = nullptr);
    ~TaskDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    // TaskView uses this method if the size of the taskview changes
    void emitSizeHintChanged(const QModelIndex &index);

public slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void generateGradientPixmap(int width, int height, QColor color, bool selected) const;

    mutable int m_cachedHeight;
    mutable QFont m_cachedFont;

    /*
      Collapsed:
      +----------------------------------------------------------------------------------------------------+
      | TASKICONAREA  TEXTAREA                         TIMEAREA DURATIONAREA RETURNAREA TIDAREA THREADSAREA|
      +----------------------------------------------------------------------------------------------------+

      Expanded:
      +----------------------------------------------------------------------------------------------------+
      | TASKICONICON  TEXTAREA                         TIMEAREA DURATIONAREA RETURNAREA TIDAREA THREADSAREA|
      |               more text -------------------------------------------------------------------------> |
      +----------------------------------------------------------------------------------------------------+
     */
    class Positions
    {
    public:
        Positions(const QStyleOptionViewItem &options, TaskModel *model) :
            totalWidth(options.rect.width()),
            maxLineLength(model->getSizeOfLineNumber(options.font)),
            iTop(options.rect.top()),
            iBottom(options.rect.bottom())
        {
            fontHeight = QFontMetrics(options.font).height();
        }

        int top() const { return iTop + ITEM_MARGIN; }
        int left() const { return ITEM_MARGIN; }
        int right() const { return totalWidth - ITEM_MARGIN; }
        int bottom() const { return iBottom; }
        int firstLineHeight() const { return fontHeight + 1; }
        static int minimumHeight() { return taskIconHeight() + 2 * ITEM_MARGIN; }

        int taskIconLeft() const { return left(); }
        static int taskIconWidth() { return TASK_ICON_SIZE; }
        static int taskIconHeight() { return TASK_ICON_SIZE; }
        int taskIconRight() const { return taskIconLeft() + taskIconWidth(); }
        QRect taskIcon() const { return QRect(taskIconLeft(), top(), taskIconWidth(), taskIconHeight()); }

        int textAreaLeft() const { return taskIconRight() + ITEM_SPACING; }
        int textAreaWidth() const { return textAreaRight() - textAreaLeft(); }
        int textAreaRight() const { return timeAreaLeft() - ITEM_SPACING; }
        QRect textArea() const { return QRect(textAreaLeft(), top(), textAreaWidth(), firstLineHeight()); }

        int timeAreaLeft() const { return timeAreaRight() - timeAreaWidth(); }
        int timeAreaWidth() const { return maxLineLength*5; }
        int timeAreaRight() const { return durationAreaLeft() - ITEM_SPACING; }
        QRect timeArea() const { return QRect(timeAreaLeft(), top(), timeAreaWidth(), firstLineHeight()); }

        int durationAreaLeft() const { return durationAreaRight() - durationAreaWidth(); }
        int durationAreaWidth() const { return maxLineLength*2; }
        int durationAreaRight() const { return returnAreaLeft() - ITEM_SPACING; }
        QRect durationArea() const { return QRect(durationAreaLeft(), top(), durationAreaWidth(), firstLineHeight()); }

        int returnAreaLeft() const { return returnAreaRight() - returnAreaWidth(); }
        int returnAreaWidth() const { return maxLineLength*3; }
        int returnAreaRight() const { return tidAreaLeft() - ITEM_SPACING; }
        QRect returnArea() const { return QRect(returnAreaLeft(), top(), returnAreaWidth(), firstLineHeight()); }

        int tidAreaLeft() const { return tidAreaRight() - tidAreaWidth(); }
        int tidAreaWidth() const { return maxLineLength; }
        int tidAreaRight() const { return threadsAreaLeft() - ITEM_SPACING; }
        QRect tidArea() const { return QRect(tidAreaLeft(), top(), tidAreaWidth(), firstLineHeight()); }

        int threadsAreaLeft() const { return threadsAreaRight() - threadsAreaWidth(); }
        int threadsAreaWidth() const { return maxLineLength; }
        int threadsAreaRight() const { return right(); }
        QRect threadsArea() const { return QRect(threadsAreaLeft(), top(), threadsAreaWidth(), firstLineHeight()); }

    private:
        int totalWidth = 0;
        int maxLineLength = 0;
        int iTop = 0;
        int iBottom = 0;
        int fontHeight = 0;

        static const int TASK_ICON_SIZE = 16;
        static const int ITEM_MARGIN = 2;
        static const int ITEM_SPACING = 2 * ITEM_MARGIN;
    };
};

TaskView::TaskView(QWidget *parent)
    : QListView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QFontMetrics fm(font());
    int vStepSize = fm.height() + 3;
    if (vStepSize < TaskDelegate::Positions::minimumHeight())
        vStepSize = TaskDelegate::Positions::minimumHeight();

    verticalScrollBar()->setSingleStep(vStepSize);
}

TaskView::~TaskView()
{ }

void TaskView::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    static_cast<TaskDelegate *>(itemDelegate())->emitSizeHintChanged(selectionModel()->currentIndex());
}

void TaskView::contextMenuEvent(QContextMenuEvent* event)
{
    // disable right context-menu
    Q_UNUSED(event)
}

/////
// TaskWindow
/////

#define TIMELINE_WIDGET_HEIGHT 60
#define STR_CURRENT_EVENT tr(" Current Event [")

class TaskWindowPrivate
{
public:
    Internal::TaskModel *taskModel = nullptr;
    Internal::TaskFilterModel *filter = nullptr;
    Internal::TaskView *listview = nullptr;
    Internal::TimelineWidget *timeline = nullptr;
    Internal::TaskWidget *widget = nullptr;
    QMenu *contextMenu = nullptr;
    QToolButton *categoriesButton = nullptr;
    QLabel* eventLabel = nullptr;
    QLineEdit* commandLine = nullptr;
    QComboBox* sortCombo = nullptr;
    QPushButton* zoomIn = nullptr;
    QPushButton* zoomOut = nullptr;
    QPushButton* zoomFit = nullptr;
    QPushButton* backward = nullptr;
    QPushButton* forward = nullptr;
    QMenu *categoriesMenu = nullptr;
    QList<QAction *> actions;
    int currentEvent = -1;
};

TaskWindow::TaskWindow()
    : d(new TaskWindowPrivate)
{
    setupUi();
}

TaskWindow::~TaskWindow()
{
    delete d->widget;
    delete d->filter;
    delete d->taskModel;
    delete d;
}

void TaskWindow::delayedInitialization()
{
}

int TaskWindow::taskCount(const QString &category) const
{
    return d->taskModel->taskCount(category);
}

QWidget *TaskWindow::outputWidget() const
{
    return d->widget;
}

QList<QWidget *> TaskWindow::toolBarWidgets() const
{
    return QList<QWidget*>() << d->categoriesButton
        << d->zoomIn
        << d->zoomOut
        << d->zoomFit
        << d->backward
        << d->forward
        << d->sortCombo
        << d->commandLine
        << d->eventLabel;
}

int TaskWindow::priorityInStatusBar() const
{
    return 90;
}

void TaskWindow::clearContents()
{
    // clear all tasks in all displays
    // Yeah we are that special
    d->taskModel->clearTasks();
    d->eventLabel->setText(STR_CURRENT_EVENT + QLatin1String("...]"));
    emit tasksCleared();
}

void TaskWindow::visibilityChanged(bool visible)
{
    if (visible)
        delayedInitialization();
}

bool TaskWindow::canFocus() const
{
    return d->filter->rowCount();
}

bool TaskWindow::hasFocus() const
{
    return d->listview->window()->focusWidget() == d->listview;
}

void TaskWindow::setFocus()
{
    if (d->filter->rowCount()) {
        d->listview->setFocus();
        if (d->listview->currentIndex() == QModelIndex())
            d->listview->setCurrentIndex(d->filter->index(0,0, QModelIndex()));
    }
}

bool TaskWindow::canNavigate() const
{
    return true;
}

bool TaskWindow::canNext() const
{
    return d->filter->rowCount();
}

bool TaskWindow::canPrevious() const
{
    return d->filter->rowCount();
}

void TaskWindow::goToNext()
{
    if (d->currentEvent + 1 < d->taskModel->rowCount())
        goTo(d->currentEvent + 1);
}

void TaskWindow::goToPrev()
{
    if (d->currentEvent > 0) goTo(d->currentEvent - 1);
}

void TaskWindow::goTo(int index)
{
    Task task = d->taskModel->task(index);
    d->eventLabel->setText( STR_CURRENT_EVENT +
            task.description + QLatin1Char(']'));
    d->currentEvent = index;
    emit coredumpChanged(index);
}

void TaskWindow::addTask(const Task &task)
{
    d->taskModel->addTask(task);

    emit tasksChanged();
    navigateStateChanged();
}

void TaskWindow::removeTask(const Task &task)
{
    d->taskModel->removeTask(task);

    emit tasksChanged();
    navigateStateChanged();
}

void TaskWindow::addCategory(const QString &categoryId, const QString &displayName, bool visible)
{
    d->taskModel->addCategory(categoryId, displayName);
    if (!visible) {
        QList<QString> filters = d->filter->filteredCategories();
        filters += categoryId;
        d->filter->setFilteredCategories(filters);
        d->timeline->setFilteredCategories(filters);
    }
}

void TaskWindow::updateTimeline(void *timeline, int count)
{
    d->timeline->setData(this, timeline, count);

    if (!timeline || !count) {
        d->taskModel->clearTasks();
        d->eventLabel->setText(STR_CURRENT_EVENT + QLatin1String("...]"));
    }
}

static bool get_range(const QString& range, int* begin, int* end)
{
    bool ok;
    int pos = range.indexOf(QLatin1Char(','));
    if (pos > 0) {
        *begin = range.left(pos).toInt(&ok, 10);
        if (!ok) return false;
        *end = range.mid(pos+1).toInt(&ok, 10);
        if (!ok) return false;
    }
    else {
        *begin = range.toInt(&ok, 10);
        if (!ok) return false;
        *end = *begin;
    }

    return (*begin >= 0 && *begin <= *end);
}

void TaskWindow::execCommand(void)
{
    int begin = -1, end = -1;
    QString cmd = d->commandLine->text();

    if (0 == cmd.indexOf(QLatin1String("sys "))) {
        if (get_range(cmd.mid(4), &begin, &end)) {
            d->filter->setEventRange(begin, end);
            d->timeline->setEventRange(begin, end);
        }
    }
    else if (0 == cmd.indexOf(QLatin1String("sig "))) {
        if (get_range(cmd.mid(4), &begin, &end)) {
            d->filter->setEventRange(
                begin + DUMP_REASON_signal,
                end + DUMP_REASON_signal);
            d->timeline->setEventRange(
                begin + DUMP_REASON_signal,
                end + DUMP_REASON_signal);
        }
    }
    else if (0 == cmd.indexOf(QLatin1String("x11 "))) {
        if (get_range(cmd.mid(4), &begin, &end)) {
            d->filter->setEventRange(
                begin + DUMP_REASON_x11,
                end + DUMP_REASON_x11);
            d->timeline->setEventRange(
                begin + DUMP_REASON_x11,
                end + DUMP_REASON_x11);
        }
    }
    else if (0 == cmd.indexOf(QLatin1String("dbus "))) {
        if (get_range(cmd.mid(5), &begin, &end)) {
            d->filter->setEventRange(
                begin + DUMP_REASON_dbus,
                end + DUMP_REASON_dbus);
            d->timeline->setEventRange(
                begin + DUMP_REASON_dbus,
                end + DUMP_REASON_dbus);
        }
    }
    else if (0 == cmd.indexOf(QLatin1String("list "))) {
        if (get_range(cmd.mid(5), &begin, &end)) {
            d->filter->setEventIndexRange(begin, end);
            d->timeline->setEventIndexRange(begin, end);
        }
    }
    else if (0 == cmd.indexOf(QLatin1String("tid "))) {
        if (get_range(cmd.mid(4), &begin, &end)) {
            d->filter->setEventTid(begin);
            d->timeline->setEventTid(begin);
        }
    }
    else if (cmd.isEmpty()) {
        d->filter->setEventIndexRange(-1, -1);
        d->timeline->setEventIndexRange(-1, -1);
    }
}

void TaskWindow::sortEvent(int index)
{
    d->filter->setSortType(index);
}

void TaskWindow::setupUi()
{
    d->taskModel = new Internal::TaskModel(this);
    d->filter = new Internal::TaskFilterModel(d->taskModel);
    d->listview = new Internal::TaskView;

    d->listview->setModel(d->filter);
    d->listview->setFrameStyle(QFrame::NoFrame);
    d->listview->setWindowTitle(tr("Events list"));
    d->listview->setSelectionMode(QAbstractItemView::SingleSelection);
    Internal::TaskDelegate *tld = new Internal::TaskDelegate(this);
    d->listview->setItemDelegate(tld);
    d->listview->setContextMenuPolicy(Qt::DefaultContextMenu);
    d->listview->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(d->listview->selectionModel(), &QItemSelectionModel::currentChanged,
            tld, &TaskDelegate::currentChanged);

    connect(d->listview->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &TaskWindow::currentChanged);
    connect(d->listview, &QAbstractItemView::activated,
            this, &TaskWindow::triggerDefaultHandler);
    connect(d->listview, &QAbstractItemView::clicked,
            this, &TaskWindow::clickItem);

    d->contextMenu = new QMenu(d->listview);

    d->widget = new TaskWidget();
    d->listview->setParent(d->widget);
    d->timeline = new TimelineWidget(d->widget);
    d->timeline->setMinimumHeight(TIMELINE_WIDGET_HEIGHT);
    d->widget->setup(d->timeline, d->listview);
    d->taskModel->setTimelinePtr(d->timeline); // mozart added.

    d->sortCombo = new QComboBox();
    d->sortCombo->addItem(tr("sort by index"));
    d->sortCombo->addItem(tr("sort by duration"));
    d->sortCombo->addItem(tr("sort by result"));
    d->sortCombo->addItem(tr("sort by number of threads"));
    connect(d->sortCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(sortEvent(int)));

    d->commandLine = new QLineEdit();
    d->commandLine->setPlaceholderText("[sys sig x11] begin[,end]");
    connect(d->commandLine, SIGNAL(returnPressed()), this, SLOT(execCommand()));

    d->zoomIn = new QPushButton(tr("zoom in"));
    connect(d->zoomIn, &QAbstractButton::clicked, d->timeline, &TimelineWidget::zoomIn);

    d->zoomOut = new QPushButton(tr("zoom out"));
    connect(d->zoomOut, &QAbstractButton::clicked, d->timeline, &TimelineWidget::zoomOut);

    d->zoomFit = new QPushButton(tr("zoom fit"));
    connect(d->zoomFit, &QAbstractButton::clicked, d->timeline, &TimelineWidget::zoomFit);

    d->zoomIn->setFlat(true);
    d->zoomOut->setFlat(true);
    d->zoomFit->setFlat(true);

    d->backward = new QPushButton();
    auto backIcon = QIcon(":/resource/images/backward_press@2x");
    d->backward->setIcon(backIcon);
    d->backward->setFlat(true);
    d->backward->setToolTip(tr("Previous Item"));
    d->backward->setFixedSize(backIcon.actualSize(backIcon.availableSizes().first()));

    connect(d->backward, &QAbstractButton::clicked, this, &TaskWindow::goToNext);

    d->forward = new QPushButton();
    auto forwardIcon = QIcon(":/resource/images/forward_press@2x");
    d->forward->setIcon(forwardIcon);
    d->forward->setFlat(true);
    d->forward->setToolTip(tr("Next Item"));
    d->forward->setFixedSize(backIcon.actualSize(backIcon.availableSizes().first()));
    connect(d->forward, &QAbstractButton::clicked, this, &TaskWindow::goToPrev);

    d->eventLabel = new QLabel(STR_CURRENT_EVENT + QLatin1String("...]"));
    d->categoriesButton = new QToolButton;
    d->categoriesButton->setIcon(QIcon(":/resource/images/filter_normal"));
    d->categoriesButton->setToolTip(tr("Filter by categories"));
    d->categoriesButton->setProperty("noArrow", true);
    d->categoriesButton->setAutoRaise(true);
    d->categoriesButton->setPopupMode(QToolButton::InstantPopup);

    d->categoriesMenu = new QMenu(d->categoriesButton);
    connect(d->categoriesMenu, &QMenu::aboutToShow, this, &TaskWindow::updateCategoriesMenu);

    d->categoriesButton->setMenu(d->categoriesMenu);

    connect(d->filter, &TaskFilterModel::rowsRemoved,
            [this]() { emit setBadgeNumber(d->filter->rowCount()); });
    connect(d->filter, &TaskFilterModel::rowsInserted,
            [this]() { emit setBadgeNumber(d->filter->rowCount()); });
    connect(d->filter, &TaskFilterModel::modelReset,
            [this]() { emit setBadgeNumber(d->filter->rowCount()); });
}

void TaskWindow::showTask(unsigned int id)
{
    int sourceRow = d->taskModel->rowForId(id);
    QModelIndex sourceIdx = d->taskModel->index(sourceRow, 0);
    QModelIndex filterIdx = d->filter->mapFromSource(sourceIdx);
    d->listview->setCurrentIndex(filterIdx);
//    popup(Core::IOutputPane::ModeSwitch);
}

void TaskWindow::openTask(unsigned int id)
{
    int sourceRow = d->taskModel->rowForId(id);
    QModelIndex sourceIdx = d->taskModel->index(sourceRow, 0);
    QModelIndex filterIdx = d->filter->mapFromSource(sourceIdx);
    triggerDefaultHandler(filterIdx);
}

void TaskWindow::clearTasks(const QString &categoryId)
{
    d->taskModel->clearTasks(categoryId);

    emit tasksChanged();
    emit tasksCleared();
    navigateStateChanged();
}

void TaskWindow::setCategoryVisibility(const QString &categoryId, bool visible)
{
    if (categoryId.isEmpty())
        return;

    QList<QString> categories = d->filter->filteredCategories();

    if (visible)
        categories.removeOne(categoryId);
    else
        categories.append(categoryId);

    d->filter->setFilteredCategories(categories);
    d->timeline->setFilteredCategories(categories);
}

void TaskWindow::currentChanged(const QModelIndex &index)
{
    qDebug() << "currentChanged " << index.row();
}

void TaskWindow::saveSettings()
{
    // do something.
}

void TaskWindow::loadSettings()
{
    // do something.
}

void TaskWindow::triggerDefaultHandler(const QModelIndex &index)
{
    bool ok;
    int i = 0;
    Task task = d->filter->task(index);
    for (; i < task.description.size(); ++i) {
        if (task.description[i] > QLatin1Char('9')) {
            break;
        }
    }

    if (i > 0) {
        i = task.description.left(i).toInt(&ok, 10);
        if (ok) {
            d->eventLabel->setText( STR_CURRENT_EVENT +
                    task.description + QLatin1Char(']'));
            d->currentEvent = i;
            emit coredumpChanged(i);
        }
    }
}

void TaskWindow::clickItem(const QModelIndex &index)
{
    qDebug() << "clickItem " << index.row();
}

void TaskWindow::actionTriggered()
{
    // do something.
}

void TaskWindow::updateCategoriesMenu()
{
    typedef QMap<QString, QString>::ConstIterator NameToIdsConstIt;

    d->categoriesMenu->clear();

    const QList<QString> filteredCategories = d->filter->filteredCategories();

    QMap<QString, QString> nameToIds;
    foreach (QString categoryId, d->taskModel->categoryIds())
        nameToIds.insert(d->taskModel->categoryDisplayName(categoryId), categoryId);

    const NameToIdsConstIt cend = nameToIds.constEnd();
    for (NameToIdsConstIt it = nameToIds.constBegin(); it != cend; ++it) {
        const QString &displayName = it.key();
        const QString categoryId = it.value();
        QAction *action = new QAction(d->categoriesMenu);
        action->setCheckable(true);
        action->setText(displayName);
        action->setChecked(!filteredCategories.contains(categoryId));
        connect(action, &QAction::triggered, this, [this, action, categoryId] {
            setCategoryVisibility(categoryId, action->isChecked());
        });
        d->categoriesMenu->addAction(action);
    }
}

/////
// Delegate
/////
TaskDelegate::TaskDelegate(QObject *parent) :
    QStyledItemDelegate(parent),
    m_cachedHeight(0)
{ }

TaskDelegate::~TaskDelegate()
{
}

QSize TaskDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QAbstractItemView * view = qobject_cast<const QAbstractItemView *>(opt.widget);
    const bool selected = (view->selectionModel()->currentIndex() == index);
    QSize s;
    s.setWidth(option.rect.width());

    if (!selected && option.font == m_cachedFont && m_cachedHeight > 0) {
        s.setHeight(m_cachedHeight);
        return s;
    }

    QFontMetrics fm(option.font);
    int fontHeight = fm.height();
    int fontLeading = fm.leading();

    TaskModel *model = static_cast<TaskFilterModel *>(view->model())->taskModel();
    Positions positions(option, model);

    if (selected) {
        QString description = index.data(TaskModel::Description).toString();
        description += QLatin1Char('\n');
        description += index.data(TaskModel::ExtraInfo).toString();
        // Layout the description
        int leading = fontLeading;
        int height = 0;
        description.replace(QLatin1Char('\n'), QChar::LineSeparator);
        QTextLayout tl(description);
        // TODO: tl.setAdditionalFormats(...);
        tl.beginLayout();
        while (true) {
            QTextLine line = tl.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(positions.textAreaWidth());
            height += leading;
            line.setPosition(QPoint(0, height));
            height += static_cast<int>(line.height());
        }
        tl.endLayout();

        s.setHeight(height + leading + fontHeight + 3);
    } else {
        s.setHeight(fontHeight + 3);
    }
    if (s.height() < positions.minimumHeight())
        s.setHeight(positions.minimumHeight());

    if (!selected) {
        m_cachedHeight = s.height();
        m_cachedFont = option.font;
    }

    return s;
}

void TaskDelegate::emitSizeHintChanged(const QModelIndex &index)
{
    emit sizeHintChanged(index);
}

void TaskDelegate::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit sizeHintChanged(current);
    emit sizeHintChanged(previous);
}

void TaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->save();

    QFontMetrics fm(opt.font);
    QColor backgroundColor;
    QColor textColor;

    const QAbstractItemView * view = qobject_cast<const QAbstractItemView *>(opt.widget);
    bool selected = view->selectionModel()->currentIndex() == index;

    if (selected) {
        painter->setBrush(opt.palette.highlight().color());
        backgroundColor = opt.palette.highlight().color();
    } else {
        painter->setBrush(opt.palette.window().color());
        backgroundColor = opt.palette.window().color();
    }
    painter->setPen(Qt::NoPen);
    painter->drawRect(opt.rect);

    // Set Text Color
    if (selected)
        textColor = opt.palette.highlightedText().color();
    else
        textColor = opt.palette.text().color();

    painter->setPen(textColor);

    TaskModel *model = static_cast<TaskFilterModel *>(view->model())->taskModel();
    Positions positions(opt, model);

    // Paint TaskIconArea:
    QIcon icon = index.data(TaskModel::Icon).value<QIcon>();
    painter->drawPixmap(positions.left(), positions.top(),
                        icon.pixmap(positions.taskIconWidth(), positions.taskIconHeight()));

    // Paint TextArea:
    if (!selected) {
        // in small mode we lay out differently
        QString bottom = index.data(TaskModel::Description).toString();
        painter->setClipRect(positions.textArea());
        painter->drawText(positions.textAreaLeft(), positions.top() + fm.ascent(), bottom);
        if (fm.horizontalAdvance(bottom) > positions.textAreaWidth()) {
            // draw a gradient to mask the text
            int gradientStart = positions.textAreaRight() - ELLIPSIS_GRADIENT_WIDTH + 1;
            QLinearGradient lg(gradientStart, 0, gradientStart + ELLIPSIS_GRADIENT_WIDTH, 0);
            lg.setColorAt(0, Qt::transparent);
            lg.setColorAt(1, backgroundColor);
            painter->fillRect(gradientStart, positions.top(), ELLIPSIS_GRADIENT_WIDTH, positions.firstLineHeight(), lg);
        }
    } else {
        // Description
        QString description = index.data(TaskModel::Description).toString();
        description += QLatin1Char('\n');
        description += index.data(TaskModel::ExtraInfo).toString();

        // Layout the description
        int leading = fm.leading();
        int height = 0;
        description.replace(QLatin1Char('\n'), QChar::LineSeparator);
        QTextLayout tl(description);
        // TODO: tl.setAdditionalFormats(...);
        tl.beginLayout();
        while (true) {
            QTextLine line = tl.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(positions.textAreaWidth());
            height += leading;
            line.setPosition(QPoint(0, height));
            height += static_cast<int>(line.height());
        }
        tl.endLayout();
        tl.draw(painter, QPoint(positions.textAreaLeft(), positions.top()));

        QColor mix;
        mix.setRgb( static_cast<int>(0.7 * textColor.red()   + 0.3 * backgroundColor.red()),
                static_cast<int>(0.7 * textColor.green() + 0.3 * backgroundColor.green()),
                static_cast<int>(0.7 * textColor.blue()  + 0.3 * backgroundColor.blue()));
        painter->setPen(mix);
    }
    painter->setPen(textColor);

    // Paint time
    double curtime = index.data(TaskModel::Time).toDouble();
    time_t seconds = static_cast<time_t>(curtime/1000.0);
    struct tm* cur_tm = localtime(&seconds);
    QString result;
    if (cur_tm) {
        result = QString::asprintf("%d/%02d/%02d %02d:%02d:%02d.%03d",
            cur_tm->tm_year + 1900, cur_tm->tm_mon + 1, cur_tm->tm_mday,
            cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec,
            int(curtime - seconds*1000.0));
    }
    int realWidth = fm.horizontalAdvance(result);
    painter->setClipRect(positions.timeArea());
    painter->drawText(qMin(positions.timeAreaLeft(), positions.timeAreaRight() - realWidth),
                      positions.top() + fm.ascent(), result);

    // Paint duration
    double duration = index.data(TaskModel::Duration).toDouble();
    result = QString::asprintf("%.3f ms", duration);
    realWidth = fm.horizontalAdvance(result);
    painter->setClipRect(positions.durationArea());
    painter->drawText(qMin(positions.durationAreaLeft(), positions.durationAreaRight() - realWidth),
                      positions.top() + fm.ascent(), result);

    // Paint syscall return
    long syscall = index.data(TaskModel::Return).toLongLong();
    if (syscall < 0xffff) {
        result = QString::asprintf("%ld", syscall);
    }
    else {
        result = QString::asprintf("%p", (void*)syscall);
    }
    realWidth = fm.horizontalAdvance(result);
    painter->setClipRect(positions.returnArea());
    painter->drawText(qMin(positions.returnAreaLeft(), positions.returnAreaRight() - realWidth),
                      positions.top() + fm.ascent(), result);
    if (realWidth > positions.returnAreaWidth()) {
        // draw a gradient to mask the text
        int gradientStart = positions.returnAreaLeft() - 1;
        QLinearGradient lg(gradientStart + ELLIPSIS_GRADIENT_WIDTH, 0, gradientStart, 0);
        lg.setColorAt(0, Qt::transparent);
        lg.setColorAt(1, backgroundColor);
        painter->fillRect(gradientStart, positions.top(), ELLIPSIS_GRADIENT_WIDTH, positions.firstLineHeight(), lg);
    }

    // Paint current tid
    int tid = index.data(TaskModel::Tid).toInt();
    result = QString::number(tid);
    painter->setClipRect(positions.tidArea());
    realWidth = fm.horizontalAdvance(result);
    painter->drawText(positions.tidAreaRight() - realWidth, positions.top() + fm.ascent(), result);

    // Paint thread num
    int threads = index.data(TaskModel::ThreadNum).toInt();
    result = QString::number(threads);
    painter->setClipRect(positions.threadsArea());
    realWidth = fm.horizontalAdvance(result);
    painter->drawText(positions.threadsAreaRight() - realWidth, positions.top() + fm.ascent(), result);

    painter->setClipRect(opt.rect);

    // Separator lines
    painter->setPen(QColor::fromRgb(150,150,150));
    painter->drawLine(0, opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    painter->restore();
}

#include "taskwindow.moc"
} // namespace Internal
} // namespace ReverseDebugger
