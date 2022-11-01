#include "CreateIssueDlg.h"
#include "ui_CreateIssueDlg.h"
#include <GitHubRestApi.h>
#include <GitLabRestApi.h>
#include <GitQlientSettings.h>
#include <GitServerCache.h>
#include <Issue.h>

#include <previewpage.h>

#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFile>
#include <QWebChannel>
#include <QDirIterator>

using namespace GitServer;

CreateIssueDlg::CreateIssueDlg(const QSharedPointer<GitServerCache> &gitServerCache, const QString &workingDir,
                               QWidget *parent)
   : QFrame(parent)
   , ui(new Ui::CreateIssueDlg)
   , mGitServerCache(gitServerCache)
{
   setAttribute(Qt::WA_DeleteOnClose);

   ui->setupUi(this);

   connect(mGitServerCache->getApi(), &IRestApi::issueUpdated, this, &CreateIssueDlg::onIssueCreated);
   connect(mGitServerCache.get(), &GitServerCache::errorOccurred, this, &CreateIssueDlg::onGitServerError);

   onMilestones(mGitServerCache->getMilestones());
   onLabels(mGitServerCache->getLabels());

   connect(ui->pbAccept, &QPushButton::clicked, this, &CreateIssueDlg::accept);

   connect(ui->teDescription, &QTextEdit::textChanged,
           [this]() { m_content.setText(ui->teDescription->toPlainText()); });

   if (QFile f(workingDir + "/.github/ISSUE_TEMPLATE.md"); f.exists())
   {
      ui->cbIssueType->setVisible(false);
      ui->lIssueType->setVisible(false);

      if (f.open(QIODevice::ReadOnly))
      {
         const auto fileContent = f.readAll();
         f.close();

         updateMarkdown(fileContent);
      }
   }
   else
   {
      connect(ui->cbIssueType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
              &CreateIssueDlg::onIssueTemplateChange);

      fillIssueTypeComboBox(workingDir);

      if (ui->cbIssueType->count() > 0)
         ui->cbIssueType->setCurrentIndex(0);
      else
      {
         ui->cbIssueType->setVisible(false);
         ui->lIssueType->setVisible(false);
      }
   }
}

CreateIssueDlg::~CreateIssueDlg()
{
   delete ui;
}

bool CreateIssueDlg::configure(const QString &workingDir)
{
   QFile f(workingDir + "/.github/ISSUE_TEMPLATE.md");

   if (f.open(QIODevice::ReadOnly))
   {
      const auto fileContent = f.readAll();
      f.close();

      GitQlientSettings settings("");
      const auto colorSchema = settings.globalValue("colorSchema", "dark").toString();
      const auto style = colorSchema == "dark" ? QString::fromUtf8("dark") : QString::fromUtf8("bright");

      PreviewPage *page = new PreviewPage(this);
      ui->preview->setPage(page);

      connect(ui->teDescription, &QTextEdit::textChanged,
              [this]() { m_content.setText(ui->teDescription->toPlainText()); });

      ui->teDescription->setText(QString::fromUtf8(fileContent));

      QWebChannel *channel = new QWebChannel(this);
      channel->registerObject(QStringLiteral("content"), &m_content);
      page->setWebChannel(channel);

      ui->preview->setUrl(QUrl(QString("qrc:/resources/index_%1.html").arg(style)));
   }

   return true;
}

void CreateIssueDlg::accept()
{
   if (ui->leTitle->text().isEmpty() || ui->teDescription->toPlainText().isEmpty())
      QMessageBox::warning(this, tr("Empty fields"), tr("Please, complete all fields with valid data."));
   else
   {
      QVector<Label> labels;

      if (const auto cbModel = qobject_cast<QStandardItemModel *>(ui->labelsListView->model()))
      {
         for (auto i = 0; i < cbModel->rowCount(); ++i)
         {
            if (cbModel->item(i)->checkState() == Qt::Checked)
            {
               Label sLabel;
               sLabel.name = cbModel->item(i)->text();
               labels.append(sLabel);
            }
         }
      }

      ui->pbAccept->setEnabled(false);

      Milestone sMilestone;
      sMilestone.id = ui->cbMilesone->count() > 0 ? ui->cbMilesone->currentData().toInt() : -1;

      GitServer::User sAssignee;
      sAssignee.name = mGitServerCache->getUserName();

      mGitServerCache->getApi()->createIssue({
          ui->leTitle->text(),
          ui->teDescription->toPlainText().toUtf8(),
          sMilestone,
          labels,
          { sAssignee },
      });
   }
}

void CreateIssueDlg::onMilestones(const QVector<Milestone> &milestones)
{
   ui->cbMilesone->addItem(tr("Select milestone"), -1);

   for (auto &milestone : milestones)
      ui->cbMilesone->addItem(milestone.title, milestone.number);

   ui->cbMilesone->setCurrentIndex(0);
}

void CreateIssueDlg::onLabels(const QVector<Label> &labels)
{
   const auto model = new QStandardItemModel(labels.count(), 0, this);
   auto count = 0;
   for (const auto &label : labels)
   {
      const auto item = new QStandardItem(label.name);
      item->setCheckable(true);
      item->setCheckState(Qt::Unchecked);
      model->setItem(count++, item);
   }
   ui->labelsListView->setModel(model);
}

void CreateIssueDlg::onIssueCreated(const GitServer::Issue &issue)
{
   QMessageBox::information(this, tr("Issue created"),
                            tr("The issue has been created. You can <a href=\"%1\">find it here</a>.").arg(issue.url));

   emit issueCreated();
}

void CreateIssueDlg::onGitServerError(const QString &error)
{
   ui->pbAccept->setEnabled(true);

   QMessageBox::warning(this, tr("API access error!"), error);
}

void CreateIssueDlg::fillIssueTypeComboBox(const QString &workingDir)
{
   QDirIterator iter(workingDir + "/.github/ISSUE_TEMPLATE/", QDir::Files);

   while (iter.hasNext())
   {
      const auto fileInfo = iter.next();

      QFile file(fileInfo);

      if (file.open(QIODevice::ReadOnly))
      {
         QByteArray line;
         do
         {
            line = file.readLine();

            if (line.contains("name:"))
            {
               line = line.remove(0, 6).trimmed();
               ui->cbIssueType->addItem(QString::fromUtf8(line), fileInfo);
               break;
            }
         } while (!line.isNull());

         file.close();
      }
   }
}

void CreateIssueDlg::onIssueTemplateChange(int newIndex)
{
   const auto fileName = ui->cbIssueType->itemData(newIndex).toString();

   if (QFile f(fileName); f.exists())
   {
      if (f.open(QIODevice::ReadOnly))
      {
         const auto fileContent = f.readAll();
         f.close();

         updateMarkdown(fileContent);
      }
   }
}

void CreateIssueDlg::updateMarkdown(const QByteArray &fileContent)
{
   GitQlientSettings settings("");
   const auto colorSchema = settings.globalValue("colorSchema", "dark").toString();
   const auto style = colorSchema == "dark" ? QString::fromUtf8("dark") : QString::fromUtf8("bright");

   PreviewPage *page = new PreviewPage(this);
   ui->preview->setPage(page);
   ui->teDescription->setText(QString::fromUtf8(fileContent));

   QWebChannel *channel = new QWebChannel(this);
   channel->registerObject(QStringLiteral("content"), &m_content);
   page->setWebChannel(channel);

   ui->preview->setUrl(QUrl(QString("qrc:/resources/index_%1.html").arg(style)));
}
