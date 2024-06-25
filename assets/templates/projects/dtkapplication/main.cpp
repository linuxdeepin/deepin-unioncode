#include <DApplication>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DApplicationSettings>
#include <DTitlebar>
#include <DProgressBar>
#include <DFontSizeManager>

#include <QPropertyAnimation>
#include <QDate>
#include <QLayout>
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    DApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dtk-application");
    a.setApplicationVersion("1.0");
    a.setProductIcon(QIcon(":/images/logo.svg"));
    a.setProductName("Dtk Application");
    a.setApplicationDescription("This is a dtk template application.");

    a.loadTranslator();
    a.setApplicationDisplayName(QCoreApplication::translate("Main", "DTK Application"));

    // 保存程序的窗口主题设置
    DApplicationSettings as;
    Q_UNUSED(as)

    DMainWindow w;
    w.titlebar()->setIcon(QIcon(":/images/logo.svg"));
    w.titlebar()->setTitle("Hello dtk");
    // 设置标题，宽度不够会隐藏标题文字
    w.setMinimumSize(QSize(600, 200));

    QWidget *cw = new QWidget(&w);
    QVBoxLayout *layout = new QVBoxLayout(cw);
    QDate today = QDate::currentDate();
    DProgressBar *yearProgressBar = new DProgressBar();
    yearProgressBar->setMaximum(today.daysInYear());
    // 绑定字体大小
    DFontSizeManager::instance()->bind(yearProgressBar, DFontSizeManager::T1);

    yearProgressBar->setAlignment(Qt::AlignCenter);
    QObject::connect(yearProgressBar, &DProgressBar::valueChanged, yearProgressBar, [yearProgressBar](int value){
        yearProgressBar->setFormat(QString("您的 %1 使用进度： %2%").arg(QDate::currentDate().year())
                                   .arg(value * 100 / yearProgressBar->maximum()));
    });

    layout->addWidget(yearProgressBar);
    w.setCentralWidget(cw);
    w.show();

    auto animation = new QPropertyAnimation(yearProgressBar, "value");
    animation->setDuration(5000);
    animation->setStartValue(0);
    animation->setEndValue(today.dayOfYear());
    animation->start();

    Dtk::Widget::moveToCenter(&w);

    return a.exec();
}
