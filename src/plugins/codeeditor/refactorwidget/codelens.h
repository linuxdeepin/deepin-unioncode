#ifndef CODELENS_H
#define CODELENS_H

#include "codelentype.h"

#include "common/common.h"

#include <QTreeView>

class CodeLens : public QTreeView
{
    Q_OBJECT
public:
    explicit CodeLens(QWidget *parent = nullptr);
    void setData(const lsp::RenameChanges &changes);
    void setData(const lsp::References &refs);

signals:
    void doubleClicked(const QString &filePath, const lsp::Range &range);
};
#endif // CODELENS_H
