#pragma once

#include <QFrame>

class FileDiffView;

class SourceCodeReview : public QFrame
{
   Q_OBJECT

public:
   explicit SourceCodeReview(const QString &filePath, const QString &sourceCode, int commentLine,
                             QWidget *parent = nullptr);

private:
   FileDiffView *mDiffView = nullptr;
};
