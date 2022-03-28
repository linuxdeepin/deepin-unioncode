#pragma once

#include <QMap>
#include <QWidget>

class GitBase;
class QTimer;
class FileEditor;
class QPushButton;
class QAbstractButton;

namespace Ui
{
class ConfigWidget;
}

class ConfigWidget : public QWidget
{
   Q_OBJECT

signals:
   void reloadView();
   void buildSystemConfigured(bool configured);
   void commitTitleMaxLenghtChanged();
   void panelsVisibilityChaned();

public:
   explicit ConfigWidget(const QSharedPointer<GitBase> &git, QWidget *parent = nullptr);
   ~ConfigWidget();

   void onPanelsVisibilityChanged();

private:
   Ui::ConfigWidget *ui;
   QSharedPointer<GitBase> mGit;
   int mOriginalRepoOrder = 0;
   bool mShowResetMsg = false;
   QTimer *mFeedbackTimer = nullptr;
   QPushButton *mSave = nullptr;
   FileEditor *mLocalGit = nullptr;
   FileEditor *mGlobalGit = nullptr;

   void clearCache();
   void calculateCacheSize();
   void toggleBsAccesInfo();
   void enableWidgets();
   void saveFile();
   void showCredentialsDlg();

private slots:
   void saveConfig();
   void onCredentialsOptionChanged(QAbstractButton *button);
};
