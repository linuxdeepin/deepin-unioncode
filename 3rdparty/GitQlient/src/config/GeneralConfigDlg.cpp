#include "GeneralConfigDlg.h"

#include <GitQlientSettings.h>
#include <GitQlientStyles.h>
#include <QLogger.h>
#include <CheckBox.h>
#include <ButtonLink.hpp>

#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

using namespace QLogger;

GeneralConfigDlg::GeneralConfigDlg(QWidget *parent)
   : QDialog(parent)
   , mSettings(new GitQlientSettings())
   , mDisableLogs(new CheckBox())
   , mLevelCombo(new QComboBox())
   , mStylesSchema(new QComboBox())
   , mGitLocation(new QLineEdit())
   , mClose(new QPushButton(tr("Close")))
   , mReset(new QPushButton(tr("Reset")))
   , mApply(new QPushButton(tr("Apply")))

{
   mGitLocation->setPlaceholderText(tr("Git location..."));

   mClose->setMinimumWidth(75);
   mReset->setMinimumWidth(75);
   mApply->setMinimumWidth(75);

   mDisableLogs->setChecked(mSettings->globalValue("logsDisabled", true).toBool());

   mLevelCombo->addItems({ "Trace", "Debug", "Info", "Warning", "Error", "Fatal" });
   mLevelCombo->setCurrentIndex(mSettings->globalValue("logsLevel", static_cast<int>(LogLevel::Warning)).toInt());

   mStylesSchema->addItems({ "dark", "bright" });

   const auto currentStyle = mSettings->globalValue("colorSchema", "dark").toString();
   mStylesSchema->setCurrentText(currentStyle);
   connect(mStylesSchema, &QComboBox::currentTextChanged, this, [this, currentStyle](const QString &newText) {
      if (newText != currentStyle)
         mShowResetMsg = true;
   });

   connect(mClose, &QPushButton::clicked, this, &GeneralConfigDlg::close);
   connect(mReset, &QPushButton::clicked, this, &GeneralConfigDlg::resetChanges);
   connect(mApply, &QPushButton::clicked, this, &GeneralConfigDlg::accept);

   const auto buttonsLayout = new QHBoxLayout();
   buttonsLayout->setContentsMargins(QMargins());
   buttonsLayout->setSpacing(20);
   buttonsLayout->addWidget(mClose);
   buttonsLayout->addStretch();
   buttonsLayout->addWidget(mReset);
   buttonsLayout->addWidget(mApply);

   auto row = 0;
   const auto layout = new QGridLayout(this);
   layout->setContentsMargins(20, 20, 20, 20);
   layout->setSpacing(20);
   layout->setAlignment(Qt::AlignTop);
   layout->addWidget(new QLabel(tr("Disable logs")), row, 0);
   layout->addWidget(mDisableLogs, row, 1);
   layout->addWidget(new QLabel(tr("Set log level")), ++row, 0);
   layout->addWidget(mLevelCombo, row, 1);
   layout->addWidget(new QLabel(tr("Styles schema")), ++row, 0);
   layout->addWidget(mStylesSchema, row, 1);
   layout->addWidget(new QLabel(tr("Git location (if not in PATH):")), ++row, 0);
   layout->addWidget(mGitLocation, row, 1);

   const auto exportLink = new ButtonLink(tr("Export config..."));
   connect(exportLink, &ButtonLink::clicked, this, &GeneralConfigDlg::exportConfig);

   layout->addWidget(exportLink, ++row, 0, 1, 2);

   const auto importLink = new ButtonLink(tr("Import config..."));
   connect(importLink, &ButtonLink::clicked, this, &GeneralConfigDlg::importConfig);

   layout->addWidget(importLink, ++row, 0, 1, 2);

   layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), ++row, 0, 1, 2);
   layout->addLayout(buttonsLayout, ++row, 0, 1, 2);

   setFixedSize(500, 300);

   setStyleSheet(GitQlientStyles::getStyles());
}

void GeneralConfigDlg::resetChanges()
{
   mDisableLogs->setChecked(mSettings->globalValue("logsDisabled", false).toBool());
   mLevelCombo->setCurrentIndex(mSettings->globalValue("logsLevel", 2).toInt());
   mStylesSchema->setCurrentText(mSettings->globalValue("colorSchema", "bright").toString());
   mGitLocation->setText(mSettings->globalValue("gitLocation", "").toString());
}

void GeneralConfigDlg::accept()
{
   mSettings->setGlobalValue("logsDisabled", mDisableLogs->isChecked());
   mSettings->setGlobalValue("logsLevel", mLevelCombo->currentIndex());
   mSettings->setGlobalValue("colorSchema", mStylesSchema->currentText());
   mSettings->setGlobalValue("gitLocation", mGitLocation->text());

   if (mShowResetMsg)
      QMessageBox::information(this, tr("Reset needed!"),
                               tr("You need to restart GitQlient to see the changes in the styles applid."));

   const auto logger = QLoggerManager::getInstance();
   logger->overwriteLogLevel(static_cast<LogLevel>(mLevelCombo->currentIndex()));

   if (mDisableLogs->isChecked())
      logger->pause();
   else
      logger->resume();

   QDialog::accept();
}

void GeneralConfigDlg::importConfig()
{
   const auto fileDialog
       = new QFileDialog(this, tr("Select a config file..."),
                         QStandardPaths::writableLocation(QStandardPaths::HomeLocation), "GitQlient.conf");
   fileDialog->setFileMode(QFileDialog::ExistingFile);

   if (fileDialog->exec())
   {
      const auto file = fileDialog->selectedFiles().constFirst();

      QFile f(file);

      if (f.open(QIODevice::ReadOnly))
      {
         QJsonDocument doc;
         doc.fromJson(f.readAll());

         const auto obj = doc.object();

         mDisableLogs->setChecked(obj[QStringLiteral("logsDisabled")].toBool());
         mLevelCombo->setCurrentIndex(obj[QStringLiteral("logsLevel")].toInt());
         mStylesSchema->setCurrentText(obj[QStringLiteral("colorSchema")].toString());
         mGitLocation->setText(obj[QStringLiteral("gitLocation")].toString());

         QMessageBox::information(this, tr("External configuration loaded!"),
                                  tr("The configuration has been loaded successfully. Remember to apply the changes."));

         f.close();
      }
   }
}

void GeneralConfigDlg::exportConfig()
{
   const auto fileDialog
       = new QFileDialog(this, tr("Select a folder..."), QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                         "GitQlient.conf");
   fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
   fileDialog->setFileMode(QFileDialog::Directory);

   if (fileDialog->exec())
   {
      QJsonObject obj;
      obj.insert("logsDisabled", mDisableLogs->isChecked());
      obj.insert("logsLevel", mLevelCombo->currentIndex());
      obj.insert("colorSchema", mStylesSchema->currentText());
      obj.insert("gitLocation", mGitLocation->text());

      QJsonDocument doc(obj);

      const auto fullPath = QString("%1/%2").arg(fileDialog->directory().path(), QString::fromUtf8("GitQlient.conf"));
      QFile f(fullPath);

      if (f.open(QIODevice::WriteOnly))
      {
         f.write(doc.toJson());
         f.close();

         QMessageBox::information(this, tr("Configuration exported!"),
                                  tr("The configuration has been stored in {%1}").arg(fullPath));
      }
   }
}
